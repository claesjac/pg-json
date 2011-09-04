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

#include "pg-json.h"

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

PG_FUNCTION_INFO_V1(json_get_value);

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