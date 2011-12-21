--TEST--
Check for get/set_option with option COUCHBASE_OPT_PREFIX_KEY
--SKIPIF--
<?php include "skipif.inc"; if(!extension_loaded("json") || !defined("COUCHBASE_SERIALIZER_JSON")) die("no json support"); ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
$key = uniqid("couchbase_");

couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, '');
var_dump(couchbase_get_option($handle, COUCHBASE_OPT_PREFIX_KEY));

@couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, array());
var_dump(couchbase_get_option($handle, COUCHBASE_OPT_PREFIX_KEY));

couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, 'prefix');
var_dump(couchbase_get_option($handle, COUCHBASE_OPT_PREFIX_KEY));

couchbase_add($handle, $key, "dummy");

couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, "prefix_1");
couchbase_add($handle, $key, "foo");
var_dump(couchbase_get($handle, $key) == "foo");

couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, "prefix");
var_dump(couchbase_get($handle, $key) == "dummy");

couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, "prefix_1");
$cas = couchbase_prepend($handle, $key, "prefix");
var_dump(couchbase_get($handle, $key) == "prefixfoo");

couchbase_cas($handle, $cas, $key, "foo");
var_dump(couchbase_get($handle, $key) == "foo");

$contents = array(
     "foo" => "dummy",
     "bar" => "dummy",
);

couchbase_set_multi($handle, $contents);
print_r(couchbase_get_multi($handle, array("foo",  "bar")));

function content_cb($res, $value) {
     print_r($value);
}

couchbase_get_delayed($handle, array("foo", "bar"), false, "content_cb");
couchbase_flush($handle);
?>
--EXPECTF--
string(0) ""
string(5) "Array"
string(6) "prefix"
bool(true)
bool(true)
bool(true)
bool(true)
Array
(
    [foo] => dummy
    [bar] => dummy
)
Array
(
    [key] => foo
    [value] => dummy
)
Array
(
    [key] => bar
    [value] => dummy
)
