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