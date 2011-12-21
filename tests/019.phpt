--TEST--
Check for couchbase delayed & callback
--SKIPIF--
<?php exit; include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$keys = array();
$cas  = array();

$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");

couchbase_set_multi($handle, $keys);

function content_cb($res, $value) {
     var_dump($res);
     print_r($value);
}

var_dump(couchbase_get_delayed($handle, array_keys($keys), false, "content_cb"));
var_dump(couchbase_get_delayed($handle, array_keys($keys), true, "content_cb"));
couchbase_flush($handle);
--EXPECTF--
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
)
bool(true)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
resource(%d) of type (Couchbase)
Array
(
    [key] => couchbase_%s
    [value] => couchbase_value_%s
    [cas] => %s
)
bool(true)
