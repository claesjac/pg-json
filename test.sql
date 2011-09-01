SELECT json_get_value('["foo", "bar"]', '[0]') = 'foo';
SELECT json_get_value('["foo", 2]', '[1]')::int = 2;
SELECT json_get_value('{"foo": "bar", "quax": "zorg"}','quax') = 'zorg';
SELECT json_get_value('{"foo": { "quax": "zorg" } }','foo.quax') = 'zorg';
SELECT json_get_value('{"foo": [{ "quax": "zorg" }] }','foo[0].quax') = 'zorg';