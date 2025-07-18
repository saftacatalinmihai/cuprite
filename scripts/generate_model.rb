require 'fileutils'
require 'time'

# Maps C types to their corresponding SQLite types.
C_TO_SQL_TYPE_MAP = {
  'int' => 'INTEGER',
  'char*' => 'TEXT',
  'float' => 'REAL',
  'double' => 'REAL'
}.freeze

# Represents a field in a C struct.
class Field
  attr_accessor :type, :name

  def initialize(type, name)
    @type = type.strip
    @name = name.strip
  end

  def pointer?
    @type.include?('*')
  end
end

# Parses the struct definition from the header file.
def parse_struct(header_content)
  struct_content = header_content[/typedef struct\s*\{([^}]+)\}/m, 1]
  return [] unless struct_content

  fields = []
  struct_content.strip.split(';').each do |line|
    line.strip!
    next if line.empty?

    match = line.match(/([\w\s\*]+)\s+(\w+)/)
    if match
      type, name = match.captures
      fields << Field.new(type, name)
    end
  end
  fields
end

def generate_migration(plural, fields)
  FileUtils.mkdir_p("db/migrations")
  timestamp = Time.now.utc.strftime('%Y%m%d%H%M%S')
  migration_file = "db/migrations/#{timestamp}_create_#{plural}.sql"

  column_defs = fields.map do |field|
    sql_type = C_TO_SQL_TYPE_MAP[field.type] || 'TEXT'
    if field.name == 'id'
      "#{field.name} #{sql_type} PRIMARY KEY"
    else
      "#{field.name} #{sql_type}"
    end
  end.join(', ')

  sql = "CREATE TABLE #{plural} (#{column_defs});"
  File.write(migration_file, sql)
  puts "Created migration: #{migration_file}"
end

def generate_new_function(klass, singular, fields)
  initializers = fields.map do |field|
    if field.pointer?
      "    m->#{field.name} = NULL;"
    else
      "    m->#{field.name} = 0;"
    end
  end.join("\n")

  <<-HEREDOC
#{klass}* #{singular}_new(void) {
    #{klass}* m = malloc(sizeof(#{klass}));
#{initializers}
    return m;
}
  HEREDOC
end

def generate_free_function(klass, singular, fields)
  free_calls = fields.select(&:pointer?).map do |field|
    "        free(m->#{field.name});"
  end.join("\n")

  <<-HEREDOC
void #{singular}_free(#{klass}* m) {
    if (m) {
#{free_calls}
        free(m);
    }
}
  HEREDOC
end

def generate_save_function(klass, singular, plural, fields)
  non_id_fields = fields.reject { |f| f.name == 'id' }
  column_names = non_id_fields.map(&:name).join(', ')
  question_marks = (['?'] * non_id_fields.length).join(', ')

  insert_binds = non_id_fields.each_with_index.map do |field, i|
    if field.type == 'char*'
      "             db_bind_text(stmt, #{i + 1}, m->#{field.name});"
    elsif field.type == 'int'
      "             db_bind_int(stmt, #{i + 1}, m->#{field.name});"
    end
  end.join("\n")

  update_set_clause = non_id_fields.map { |f| "#{f.name} = ?" }.join(', ')
  update_binds = non_id_fields.each_with_index.map do |field, i|
    if field.type == 'char*'
      "             db_bind_text(stmt, #{i + 1}, m->#{field.name});"
    elsif field.type == 'int'
      "             db_bind_int(stmt, #{i + 1}, m->#{field.name});"
    end
  end.join("\n")
  update_binds << "\n             db_bind_int(stmt, #{non_id_fields.length + 1}, m->id);"

  <<-HEREDOC
int #{singular}_save(#{klass}* m) {
    if (m->id == 0) {
        // Create
        const char* sql = "INSERT INTO #{plural} (#{column_names}) VALUES (#{question_marks})";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
#{insert_binds}
            db_step(stmt);
            m->id = sqlite3_last_insert_rowid(db_handle());
            db_finalize(stmt);
        }
    } else {
        // Update
        const char* sql = "UPDATE #{plural} SET #{update_set_clause} WHERE id = ?";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
#{update_binds}
            db_step(stmt);
            db_finalize(stmt);
        }
    }
    return m->id;
}
  HEREDOC
end

def generate_find_function(klass, singular, plural, fields)
  column_names = fields.map(&:name).join(', ')
  assignments = fields.each_with_index.map do |field, i|
    if field.type == 'char*'
      "            const unsigned char* #{field.name} = db_column_text(stmt, #{i});
            if (#{field.name}) {
                m->#{field.name} = strdup((const char *)#{field.name});
            }"
    else
      "            m->#{field.name} = db_column_int(stmt, #{i});"
    end
  end.join("\n")

  <<-HEREDOC
#{klass}* #{singular}_find(int id) {
    #{klass}* m = NULL;
    const char* sql = "SELECT #{column_names} FROM #{plural} WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);

    if (stmt) {
        if (db_bind_int(stmt, 1, id) != SQLITE_OK) {
            fprintf(stderr, "Failed to bind id: %s\\n", sqlite3_errmsg(db_handle()));
            db_finalize(stmt);
            return NULL;
        }
        
        int r = db_step(stmt);
        if (r == SQLITE_ROW) {
            m = #{singular}_new();
#{assignments}
        }
        db_finalize(stmt);
    }
    return m;
}
  HEREDOC
end
def generate_all_function(klass, singular, plural, fields)
  column_names = fields.map(&:name).join(', ')
  assignments = fields.each_with_index.map do |field, i|
    if field.type == 'char*'
      "        const unsigned char* val_#{field.name} = db_column_text(stmt, #{i});
        if (val_#{field.name}) {
            m->#{field.name} = strdup((const char *)val_#{field.name});
        }"
    else
      "        m->#{field.name} = db_column_int(stmt, #{i});"
    end
  end.join("\n")

  <<-HEREDOC
#{klass}** #{singular}_all(int* count) {
    const char* sql = "SELECT #{column_names} FROM #{plural}";
    sqlite3_stmt* stmt = db_prepare(sql);
    
    *count = 0;
    if (!stmt) {
        return NULL;
    }

    int capacity = 0;
    #{klass}** models = NULL;

    while (db_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            #{klass}** new_models = realloc(models, sizeof(#{klass}*) * capacity);
            if (!new_models) {
                fprintf(stderr, "Failed to reallocate memory for models\\n");
                for(int i = 0; i < *count; i++) {
                    #{singular}_free(models[i]);
                }
                free(models);
                db_finalize(stmt);
                return NULL;
            }
            models = new_models;
        }
        #{klass}* m = #{singular}_new();
#{assignments}
        models[*count] = m;
        (*count)++;
    }
    db_finalize(stmt);

    return models;
}
  HEREDOC
end

def generate_model(name)
  singular = name.downcase
  plural = singular + 's'
  klass = name.capitalize

  header_path = "app/models/#{singular}.h"
  unless File.exist?(header_path)
    puts "Error: #{header_path} not found."
    exit 1
  end
  header_content = File.read(header_path)
  fields = parse_struct(header_content)

  if fields.empty?
    puts "Error: Could not parse struct definition in #{header_path}"
    exit 1
  end

  FileUtils.mkdir_p("app/models/generated")

  generate_migration(plural, fields)

  h_file = <<-HEREDOC
#ifndef #{klass.upcase}_GENERATED_H
#define #{klass.upcase}_GENERATED_H

#include "../#{singular}.h"

#{klass}* #{singular}_new(void);
void #{singular}_free(#{klass}* m);
int #{singular}_save(#{klass}* m);
#{klass}* #{singular}_find(int id);
#{klass}** #{singular}_all(int* count);
void #{singular}_destroy(int id);

#endif
  HEREDOC

  c_file = <<-HEREDOC
#include "#{singular}.h"
#include "db/db.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#{generate_new_function(klass, singular, fields)}
#{generate_free_function(klass, singular, fields)}
#{generate_save_function(klass, singular, plural, fields)}
#{generate_find_function(klass, singular, plural, fields)}
#{generate_all_function(klass, singular, plural, fields)}

void #{singular}_destroy(int id) {
    const char* sql = "DELETE FROM #{plural} WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);
    if (stmt) {
        db_bind_int(stmt, 1, id);
        db_step(stmt);
        db_finalize(stmt);
    }
}
  HEREDOC

  File.write("app/models/generated/#{singular}.h", h_file)
  File.write("app/models/generated/#{singular}.c", c_file)
end

if ARGV.empty?
  puts "Usage: ruby generate_model.rb <model_name>"
  exit 1
end

generate_model(ARGV[0])