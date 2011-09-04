-- Selection by path
SELECT json_get_value('["foo", "bar"]', '[0]') = 'foo';
SELECT json_get_value('["foo", 2]', '[1]')::int = 2;
SELECT json_get_value('{"foo": "bar", "quax": "zorg"}','quax') = 'zorg';
SELECT json_get_value('{"foo": { "quax": "zorg" } }','foo.quax') = 'zorg';
SELECT json_get_value('{"foo": [{ "quax": "zorg" }] }','foo[0].quax') = 'zorg';

SELECT '{"foo":"bar"}'::json ~ 'foo' = 'bar';

-- Equality
SELECT '{"foo":"bar"}'::json = '{"foo":"bar"}'::json;
SELECT '[1,2,3]'::json = '[1,2,3]'::json;
SELECT '{"foo": [1,2,3]}'::json = '{"foo": [1,2,3]}'::json;
SELECT '{"a": "b", "c": "d"}'::json = '{"c": "d", "a": "b"}'::json;

SELECT '{"foo": [1,2,3]}'::json != '{"foo": [2,3,4]}'::json;