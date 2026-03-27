#include "todos_controller.h"
#include "application_controller.h"
#include "models/generated/todo.h"

#include "http.h" /* the HTTP facil.io extension */
#include "fiobj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void private_render_todos_view(http_s* request, char* view, Todo* t) {
    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("text", 4), fiobj_str_new(t->text, strlen(t->text)));
    fiobj_hash_set(data, fiobj_str_new("id", 2), fiobj_num_new(t->id));
    fiobj_hash_set(data, fiobj_str_new("completed", 9), fiobj_num_new(t->completed));
    render(request, view, data);
    fiobj_free(data);
    return ;
}

Todo* find_todo_by_request_id(http_s* request) {
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2));
    if (!id_obj) {
        http_send_error(request, 404);
        return NULL;
    }
    Todo* t = todo_find((int)fiobj_obj2num(id_obj));
    if (!t) {
        http_send_error(request, 404);
        return NULL;
    }
    return t;
}

void todos_index(http_s* request) {
    int count = 0;
    Todo** todos = todo_all(&count);

    FIOBJ todos_ary = fiobj_ary_new();
    for (int i = 0; i < count; i++) {
        FIOBJ todo_hash = fiobj_hash_new();
        fiobj_hash_set(todo_hash, fiobj_str_new("text", 4), fiobj_str_new(todos[i]->text, strlen(todos[i]->text)));
        fiobj_hash_set(todo_hash, fiobj_str_new("id", 2), fiobj_num_new(todos[i]->id));

        FIOBJ completed_bool; 
        if (todos[i]->completed) {
            completed_bool = fiobj_true();
        } else {
            completed_bool = fiobj_false();
        }
        fiobj_hash_set(todo_hash, fiobj_str_new("completed", 9), completed_bool);
        fiobj_ary_push(todos_ary, todo_hash);
    }

    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("todos", 5), todos_ary);
    render(request, "todos/index", data);
    fiobj_free(data);

    for (int i = 0; i < count; i++) {
        todo_free(todos[i]);
    }
    free(todos);
}

void todos_show(http_s* request) {
    Todo* t = find_todo_by_request_id(request);
    if (!t) {
        return;
    }
    private_render_todos_view(request, "todos/show", t);
    todo_free(t);
}

void todos_new(http_s* request){
    FIOBJ data = fiobj_hash_new();
    render(request, "todos/new", data);
    fiobj_free(data);
}

void todos_create(http_s* request) {
    if (!request->body) {
        http_send_error(request, 400);
        return;
    }

    fio_str_info_s body = fiobj_data_gets(request->body);
    strtok(body.data, "=\n"); // consumes the "name" form key
    char* todo_text = strtok(NULL, "=\n"); // gents the value of the name form key 

    Todo* t = todo_new();
    t->text = todo_text;
    
    int id = todo_save(t);
    printf("Created and saved new todo with id: %d\n", id);
    
    private_render_todos_view(request, "todos/show", t);
    // product_free(p); // THIS fails for some reason... don't know why.
}

void todos_edit(http_s* request) {
    Todo* t = find_todo_by_request_id(request);
    if (!t) {
        return;
    }

    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("text", 4), fiobj_str_new(t->text, strlen(t->text)));
    fiobj_hash_set(data, fiobj_str_new("id", 2), fiobj_num_new(t->id));
    fiobj_hash_set(data, fiobj_str_new("completed", 9), fiobj_num_new(t->completed));
    render(request, "todos/edit", data);
    fiobj_free(data);
}


void todos_update(http_s* request) {
    Todo* t = find_todo_by_request_id(request);
    if (!t) {
        return;
    }
    
    fio_str_info_s body = fiobj_data_gets(request->body);
    strtok(body.data, "=\n"); // consumes the "text" form key
    char* todo_text = strtok(NULL, "=\n"); // gents the value of the text form key 
    char* todo_completed = strtok(NULL, "=\n"); // gents the value of the completed form key 

    t->text = todo_text;
    t->completed = atoi(todo_completed);
    todo_save(t);
    private_render_todos_view(request, "todos/show", t);
    return;
}

void todos_destroy(http_s* request) {
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2));
    if (!id_obj) {
        http_send_error(request, 404);
        return ;
    }
    int id = (int)fiobj_obj2num(id_obj);
    todo_destroy(id);
    todos_index(request);
    return ;
}