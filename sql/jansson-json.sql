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
 
CREATE OR REPLACE FUNCTION json_in(cstring)
    RETURNS json
    AS '$libdir/jansson-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_out(json)
    RETURNS cstring
    AS '$libdir/jansson-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_get_value(data json, path text)
    RETURNS text
    AS '$libdir/jansson-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_set_value(data json, path text, value json)
    RETURNS json
    AS '$libdir/jansson-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_equals(this json, that json)
    RETURNS boolean
    AS '$libdir/jansson-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_not_equals(this json, that json)
        RETURNS boolean
        AS '$libdir/jansson-json'
        LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_concat(this json, that json)
        RETURNS json
        AS '$libdir/jansson-json'
        LANGUAGE C IMMUTABLE STRICT;
        
CREATE TYPE json (
    INPUT = json_in,
    OUTPUT = json_out
);

CREATE OPERATOR ~ (
    PROCEDURE = json_get_value,
    LEFTARG = json, RIGHTARG = text
);

CREATE OPERATOR = (
    PROCEDURE = json_equals,
    LEFTARG = json, RIGHTARG = json,
    COMMUTATOR = =,
    NEGATOR = !=
);

CREATE OPERATOR != (
    PROCEDURE = json_not_equals,
    LEFTARG = json, RIGHTARG = json,
    COMMUTATOR = !=,
    NEGATOR = =
);

CREATE OPERATOR || (
    PROCEDURE = json_concat,
    LEFTARG = json, RIGHTARG = json
);