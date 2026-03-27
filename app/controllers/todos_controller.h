#ifndef TODOS_CONTROLLER_H
#define TODOS_CONTROLLER_H

#include "http.h"
#include "cuprite.h"
#include "models/generated/todo.h"

#include "http.h" /* the HTTP facil.io extension */
#include "fiobj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void todos_index(http_s* request);
void todos_create(http_s* request);
void todos_update(http_s* request);
void todos_destroy(http_s* request);
void todos_clear_completed(http_s* request);

void private_render_todos_view(http_s* request, char* view, Todo* t) {
    FIOBJ data = todo_to_fiobj(t);
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
    Todo** reversed = malloc(sizeof(Todo*) * count);
    for (int i = 0; i < count; i++) {
        reversed[i] = todos[count - 1 - i];
    }
    free(todos);
    todos = reversed;
    FIOBJ data = todos_to_fiobj(todos, count);

    render(request, "todos/index", data);

    fiobj_free(data);
    for (int i = 0; i < count; i++) {
        todo_free(todos[i]);
    }
    free(todos);
}

void todos_create(http_s* request) {
    if (!request->body) {
        http_send_error(request, 400);
        return;
    }

    fio_str_info_s body = fiobj_data_gets(request->body);
    printf("body: %s\n", body.data);
    if (!body.data) {
        http_send_error(request, 400);
        return;
    }
    strtok(body.data, "=\n"); // consume the text key
    char* todo_text = strtok(NULL, "=\n");
    char* decoded_text = malloc(strlen(todo_text) + 1);
    url_decode(decoded_text, todo_text);

    Todo* t = todo_new();
    t->text = decoded_text;
    t->completed = false;
    int id = todo_save(t);
    printf("Created and saved new todo with id: %d\n", id);
    private_render_todos_view(request, "todos/todo", t);
    todo_free(t);
}

void todos_update(http_s* request) {
    Todo* t = find_todo_by_request_id(request);
    if (!t) {
        return;
    }

    fio_str_info_s body = fiobj_data_gets(request->body);
    printf("body: %s\n", body.data);
    if (!body.data) {
        http_send_error(request, 400);
        return;
    }
    char* key = strtok(body.data, "=\n");
    while(key) {
        if (strcmp(key, "completed") == 0) {
            char* todo_completed = strtok(NULL, "=\n");
            if (strcmp(todo_completed, "true") == 0) {
                printf("Setting completed to true\n");
                t->completed = true;
            } else {
                printf("Setting completed to false\n");
                t->completed = false;
            }
        } 
        if (strcmp(key, "text") == 0) {
            char* todo_text = strtok(NULL, "=\n"); 
            char* decoded_text = malloc(strlen(todo_text) + 1);
            url_decode(decoded_text, todo_text);
            t->text = decoded_text;
        }
        key = strtok(NULL, "=\n");
    }
    todo_save(t);
    private_render_todos_view(request, "todos/todo", t);
    todo_free(t);
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
    http_send_body(request, NULL, 0);
    return ;
}

void todos_clear_completed(http_s* request) {
    int count = 0;
    Todo** todos = todo_all(&count);
    
    // Delete all completed todos
    for (int i = 0; i < count; i++) {
        if (todos[i]->completed) {
            todo_destroy(todos[i]->id);
        }
        todo_free(todos[i]);
    }
    free(todos);
    
    // Render the updated todos index
    todos_index(request);
    return;
}

#endif