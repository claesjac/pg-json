/*-------------------------------------------------------------------------
 *
 * JSON functions using the Jansson JSON library
 *
 * Copyright (c) 2011, Claes Jakobsson, Glue Finance AB
 *
 * This software is licensed under the MIT license. See LICENSE
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <jansson.h>

typedef struct varlena Json;

PG_MODULE_MAGIC;

Datum json_in(PG_FUNCTION_ARGS);
Datum json_out(PG_FUNCTION_ARGS);

Datum json_get_value(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/
PG_FUNCTION_INFO_V1(json_in);

Datum json_in(PG_FUNCTION_ARGS) {
    char            *str = PG_GETARG_CSTRING(0);
    int4            len = strlen(str);
    Json            *result;
    json_t          *root;
    json_error_t    error;
    
    /* Parse to see if this is valid JSON */
    root = json_loads(str, 0, &error);
    if (!root) {
        elog(ERROR, "Failed to parse json: %s", error.text);
    }
    json_decref(root);
    
    result = (Json *) palloc(len + VARHDRSZ);
    SET_VARSIZE(result, len + VARHDRSZ);
    memcpy(VARDATA(result), str, len);
    
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(json_out);

Datum json_out(PG_FUNCTION_ARGS) {
    Json    *json = (Json *) PG_GETARG_POINTER(0);
    size_t  len = VARSIZE_ANY_EXHDR(json);
    char    *result;

    result = (char *) palloc(len + 1);
    memcpy(result, VARDATA_ANY(json), len);
    result[len] = '\0';
    
    PG_RETURN_CSTRING(result);
}

static json_t *get_value_at_path(json_t *obj, const char *path) {
    long ix = 0, offset = 0;
    json_t *rv = NULL;
    char *key;
    
    /* No more path to extract */
    if (*path == '\0') {
        return obj;
    }
    
    if (*path == '[') {
        if (!json_is_array(obj)) {
            elog(ERROR, "Object is not an array for path '%s'", path);
            return NULL;
        }

        offset = 1;
        while (isdigit(path[offset])) {
            offset++;
        }
        if (offset == 1) {
            elog(ERROR, "Missing array index");
            return NULL;
        }
        if (path[offset] != ']') {
            elog(ERROR, "Invalid path, expected ']' but got '%c'", path[offset]);
            return NULL;
        }
        
        ix = strtol(path + 1, NULL, 10);
        
        if (ix >= json_array_size(obj)) {
            elog(ERROR, "Subscript out of range: %d > %d", (int) ix, (int) json_array_size(obj) - 1);
            return NULL;        
        }
        
        rv = json_array_get(obj, ix);
        if (rv == NULL) {
            return NULL;            
        }
        
        return get_value_at_path(rv, path + offset + 1);
    }
    else {        
        /* Skip separators */
        if (*path == '.') { path++; }
        
        while(path[offset] != '\0' && path[offset] != '[' && path[offset] != '.') {
            offset++;
        }
        
        if (offset == 0) {
            elog(INFO, "No key");
            return NULL;
        }
        
        key = calloc(offset + 1, sizeof(char));
        strncpy(key, path, offset);        
                
        rv = json_object_get(obj, key);
        free(key);

        if (rv == NULL) {
            return NULL;
        }
        
        return get_value_at_path(rv, path + offset);
    }

    return NULL;
}

static text *json_t_to_text(json_t *rv) {
    char buffer[32];
    
    switch (json_typeof(rv)) {
        case JSON_OBJECT:
        case JSON_ARRAY:
            /* Stringify back */
            return cstring_to_text("[ object Object ]");
            break;
        
        case JSON_STRING:
            return cstring_to_text(json_string_value(rv));
            break;
        
        case JSON_INTEGER:
#if JSON_INTEGER_IS_LONG_LONG
            snprintf(buffer, sizeof(buffer), "%lld", json_integer_value(rv));
#else
            snprintf(buffer, sizeof(buffer), "%ld", json_integer_value(rv));
#endif
            return cstring_to_text(buffer);
            break;
            
        case JSON_REAL:
            break;
        
        case JSON_TRUE:
        case JSON_FALSE:
            return cstring_to_text(json_is_true(rv) ? "true" : "false");
            break;
        
        default:
            elog(INFO, "Unknown JSON type: %d", json_typeof(rv));
    }
    
    return NULL;
}

PG_FUNCTION_INFO_V1(json_get_value);

Datum json_get_value(PG_FUNCTION_ARGS)
{
    Json*   json = (Json *) PG_GETARG_POINTER(0);
    size_t  len = VARSIZE_ANY_EXHDR(json);
    json_t*     root;
    json_t*     rv;
    json_error_t error;
    text        *t;
    
    root = json_loadb(VARDATA(json), len, 0, &error);
    if (!root) {
        elog(ERROR, "Failed to parse json: %s", error.text);
    }

    rv = get_value_at_path(root, (const char *) text_to_cstring(PG_GETARG_TEXT_P(1)));    

    /* Return nothing */
    if (rv == NULL || json_is_null(rv)) {
        PG_RETURN_NULL();
    }
    
    t = (text *) json_t_to_text(rv);
    json_decref(rv);
    if (t == NULL) {
        PG_RETURN_NULL();
    }

    PG_RETURN_TEXT_P(t);
}

PG_FUNCTION_INFO_V1(json_set_value);

static void set_value_at_path(json_t *obj, const char *path, json_t *new_v) {
    long ix = 0, offset = 0;
    char *key;
    const char *orig_path = path;
    
    if (*path == '[') {
        if (!json_is_array(obj)) {
            elog(ERROR, "Object is not an array for path '%s'", path);
            return;
        }

        offset = 1;
        while (isdigit(path[offset])) {
            offset++;
        }
        if (offset == 1) {
            elog(ERROR, "Missing array index");
            return;
        }
        if (path[offset] != ']') {
            elog(ERROR, "Invalid path, expected ']' but got '%c'", path[offset]);
            return;
        }
        
        ix = strtol(path + 1, NULL, 10);
        
        if (ix >= json_array_size(obj)) {
            elog(ERROR, "Subscript out of range: %d > %d", (int) ix, (int) json_array_size(obj) - 1);
            return;        
        }
        
        if (path[offset + 1] == '\0') {
            json_array_set(obj, ix, new_v);
            return;
        }
        else {                
            obj = json_array_get(obj, ix);
            if (obj == NULL) {
                return;            
            }
            set_value_at_path(obj, path + offset + 1, new_v);
        }
    }
    else {        
        /* Skip separators */
        if (*path == '.') { path++; }
        
        while(path[offset] != '\0' && path[offset] != '[' && path[offset] != '.') {
            offset++;
        }
        
        if (offset == 0) {
            elog(ERROR, "No key at %s:%d", __FILE__, __LINE__);
            return;
        }
        
        key = calloc(offset + 1, sizeof(char));
        strncpy(key, path, offset);        

        if (path[offset] == '\0') {
            if (json_object_set_new(obj, key, new_v) != 0) {
                elog(ERROR, "Failed to set key '%s'", key);
                free(key);
                return;
            }
        }
        else {
            obj = json_object_get(obj, key);
            free(key);
            
            if (obj == NULL) {
                return;
            }
        
            set_value_at_path(obj, path + offset, new_v);
        }
    }
}

Datum json_set_value(PG_FUNCTION_ARGS)
{
    Json*       json;
    size_t      len;
    json_t*     root;
    json_t*     sv;
    char*       str;
        
    json_error_t error;
    
    json = (Json *) PG_GETARG_POINTER(0);
    len = VARSIZE_ANY_EXHDR(json);
    root = json_loadb(VARDATA(json), len, 0, &error);
    if (!root) {
        elog(ERROR, "Failed to parse json: %s", error.text);
    }

    json = (Json *) PG_GETARG_POINTER(2);
    len = VARSIZE_ANY_EXHDR(json);
    sv = json_loadb(VARDATA(json), len, 0, &error);
    if (!sv) {
        elog(ERROR, "Failed to parse value: %s", error.text);
    }

    set_value_at_path(root, (const char *) text_to_cstring(PG_GETARG_TEXT_P(1)), sv);    

    str = json_dumps(root, JSON_COMPACT);

    /* Avoid leaks */
    json_decref(root);
    json_decref(sv);

    /* Convert back to a json type */
    len = strlen(str);
    json = (Json *) palloc(len + VARHDRSZ);
    SET_VARSIZE(json, len + VARHDRSZ);
    memcpy(VARDATA(json), str, len);

    free(str);

    PG_RETURN_POINTER(json);
}

/* Comparision */
static int json_compare(Json *ja, Json *jb) {
    size_t  len;
    json_t  *a;
    json_t  *b;
    json_error_t error;

    len = VARSIZE_ANY_EXHDR(ja);
    a = json_loadb(VARDATA(ja), len, 0, &error);
    if (!a) {
        elog(ERROR, "Failed to parse LH json: %s", error.text);
        return 0;
    }
 
    len = VARSIZE_ANY_EXHDR(jb); 
    b = json_loadb(VARDATA(jb), len, 0, &error);
    if (!b) {
        elog(ERROR, "Failed to parse RH json: %s", error.text);
        return 0;
    }    
    
    return json_equal(a, b);
}

PG_FUNCTION_INFO_V1(json_equals);
Datum json_equals(PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL(json_compare((Json *) PG_GETARG_POINTER(0), (Json *) PG_GETARG_POINTER(1)) == 1);
}

PG_FUNCTION_INFO_V1(json_not_equals);
Datum json_not_equals(PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL(json_compare((Json *) PG_GETARG_POINTER(0), (Json *) PG_GETARG_POINTER(1)) != 1);
}

/* Concatenation */

PG_FUNCTION_INFO_V1(json_concat);
Datum json_concat(PG_FUNCTION_ARGS) {
    Json *d;
    json_t *a;
    json_t *b;
    json_error_t error;
    char *str;
    size_t len, i;

    d = (Json *) PG_GETARG_POINTER(0);
    a = json_loadb(VARDATA(d), VARSIZE_ANY_EXHDR(d), 0, &error);
    if (!a) {
        elog(ERROR, "Failed to parse LH json: %s", error.text);
        PG_RETURN_NULL();
    }

    d = (Json *) PG_GETARG_POINTER(1);
    b = json_loadb(VARDATA(d), VARSIZE_ANY_EXHDR(d), 0, &error);
    if (!b) {
        elog(ERROR, "Failed to parse RH json: %s", error.text);
        PG_RETURN_NULL();
    }
    
    if (json_is_object(a)) {
        if (!json_is_object(b)) {
            elog(ERROR, "Can't concat a non-object with an object");
            PG_RETURN_NULL();
        }
        
        void *iter = json_object_iter(b);
        while (iter != NULL) {
            json_object_set(a, json_object_iter_key(iter), json_object_iter_value(iter));
            iter = json_object_iter(iter);
        }
    }
    else if (json_is_array(a)) {
        if (!json_is_array(b)) {
            elog(ERROR, "Can't concat a non-array with an array");
            PG_RETURN_NULL();
        }
        
        len = json_array_size(b);
        for (i = 0; i < len; i++) {
            json_array_append(a, json_array_get(b, i));
        }
    }
    else {
        elog(ERROR, "Invalid combination of types to concat, must be object-object or array-array");
        PG_RETURN_NULL();
    }
    
    str = json_dumps(a, JSON_COMPACT);

    /* Avoid leaks */
    json_decref(a);
    json_decref(b);

    /* Convert back to a json type */
    len = strlen(str);
    d = (Json *) palloc(len + VARHDRSZ);
    SET_VARSIZE(d, len + VARHDRSZ);
    memcpy(VARDATA(d), str, len);

    free(str);
    
    PG_RETURN_POINTER(d);
}
