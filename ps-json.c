/*-------------------------------------------------------------------------
 *
 * JSON functions using the Jansson JSON library
 *
 * Copyright (c) 2011, Claes Jakobsson
 *
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include <jansson.h>

PG_MODULE_MAGIC;


Datum		json_get_value(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(json_get_value);

Datum
json_get_value(PG_FUNCTION_ARGS)
{
    const char *json = text_to_cstring(PG_GETARG_TEXT_P(0));
    const char *path = text_to_cstring(PG_GETARG_TEXT_P(1));
    json_t *root;
    json_error_t error;
    
    elog(ERROR, "Got here");
    
    root = json_loads(json, 0, &error);
    if (!root) {
        elog(ERROR, "Failed to parse json: %s", error.text);
    }
    
    PG_RETURN_TEXT_P(PG_GETARG_TEXT_P(0));
}
