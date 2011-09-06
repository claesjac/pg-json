-- Selection by path
SELECT json_get_value('["foo", "bar"]', '[0]') = 'foo';
SELECT json_get_value('["foo", 2]', '[1]')::int = 2;
SELECT json_get_value('{"foo": "bar", "quax": "zorg"}','quax') = 'zorg';
SELECT json_get_value('{"foo": { "quax": "zorg" } }','foo.quax') = 'zorg';
SELECT json_get_value('{"foo": [{ "quax": "zorg" }] }','foo[0].quax') = 'zorg';

-- ~ is the  operator version of json_get_value
SELECT '{"foo":"bar"}'::json ~ 'foo' = 'bar';

-- Equality
SELECT json_equals('{"foo":"bar"}', '{"foo":"bar"}');

-- = is the operator version of json_equals
SELECT '{"foo":"bar"}'::json = '{"foo":"bar"}'::json;
SELECT '[1,2,3]'::json = '[1,2,3]'::json;
SELECT '{"foo": [1,2,3]}'::json = '{"foo": [1,2,3]}'::json;
SELECT '{"a": "b", "c": "d"}'::json = '{"c": "d", "a": "b"}'::json;

SELECT json_not_equals('{"foo":"bar"}', '{"foo2":"bar2"}');

-- != is the operator version of json_not_equals
SELECT '{"foo": [1,2,3]}'::json != '{"foo": [2,3,4]}'::json;

-- Concatenation
SELECT json_concat('{"a":"b"}', '{"c":"d"}') = '{"a":"b","c":"d"}';
SELECT json_concat('[1, 2, 3]','[4, 5, 6]') = '[1, 2, 3, 4, 5, 6]';

SELECT '{"a":"b"}'::json || '{"c":"d"}'::json = '{"a":"b","c":"d"}'::json;
SELECT '[1, 2, 3]'::json || '[4, 5, 6]'::json = '[1, 2, 3, 4, 5, 6]'::json;

