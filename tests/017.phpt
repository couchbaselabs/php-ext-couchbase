--TEST--
Check for couchbase_set_multi
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
$values = array();
$i = 0;
while (++$i < 10) {
   $values[uniqid("couchbase_")] = "foo";
}

print_r(couchbase_set_multi($handle, $values, 1));

couchbase_flush($handle);
?>
--EXPECTF--
Array
(
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
    [couchbase_%s] => %s
)
