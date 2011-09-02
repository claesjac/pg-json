-- Adjust this setting to control where the objects get created.
SET search_path = public;

CREATE OR REPLACE FUNCTION json_in(cstring)
    RETURNS json
    AS '$libdir/pg-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_out(json)
    RETURNS cstring
    AS '$libdir/pg-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION json_get_value(data json, path text)
    RETURNS text
    AS '$libdir/pg-json'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE json (
    INPUT = json_in,
    OUTPUT = json_out
);

CREATE OPERATOR ~ (
    PROCEDURE = json_get_value,
    LEFTARG = json, RIGHTARG = text
);
