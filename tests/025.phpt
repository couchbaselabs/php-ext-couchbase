--TEST--
Check for get/set_option with option COUCHBASE_OPT_SERIALIZER
--SKIPIF--
<?php include "skipif.inc"; if(!extension_loaded("json") || !defined("COUCHBASE_SERIALIZER_JSON")) die("no json support"); ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

couchbase_set_option($handle, COUCHBASE_OPT_SERIALIZER, COUCHBASE_SERIALIZER_JSON);

$key = uniqid("couchbase_");

$value = array(1, 2, 3, "test" => "bar", array("dummy"));

couchbase_add($handle, $key, $value);
print_r(couchbase_get($handle, $key));
couchbase_set_option($handle, COUCHBASE_OPT_SERIALIZER, COUCHBASE_SERIALIZER_JSON_ARRAY);
print_r(couchbase_get($handle, $key));

couchbase_delete($handle, $key);
?>
--EXPECTF--
stdClass Object
(
    [0] => 1
    [1] => 2
    [2] => 3
    [test] => bar
    [3] => Array
        (
            [0] => dummy
        )

)
Array
(
    [0] => 1
    [1] => 2
    [2] => 3
    [test] => bar
    [3] => Array
        (
            [0] => dummy
        )

)
