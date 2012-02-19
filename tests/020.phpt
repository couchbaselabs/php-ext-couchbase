--TEST--
Check for couchbase delayed & fetch_all
--SKIPIF--
<?php include "skipif.inc" ?>
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

couchbase_set_multi($handle, $keys);

var_dump(couchbase_get_delayed($handle, array_keys($keys), false));
print_r(couchbase_fetch_all($handle));
print_r(couchbase_fetch_all($handle));
var_dump(couchbase_get_delayed($handle, array_keys($keys), true));
print_r(couchbase_fetch_all($handle));
couchbase_flush($handle);
--EXPECTF--
bool(true)
Array
(
    [0] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
        )

    [1] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
        )

    [2] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
        )

    [3] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
        )

)
bool(true)
Array
(
    [0] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
            [cas] => %s
        )

    [1] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
            [cas] => %s
        )

    [2] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
            [cas] => %s
        )

    [3] => Array
        (
            [key] => couchbase_%s
            [value] => couchbase_value_%s
            [cas] => %s
        )

)
