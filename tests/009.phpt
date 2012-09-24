--TEST--
Check for expiration and touch support
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";

couchbase_add($handle, $key, $value, 1);
var_dump(couchbase_get($handle, $key));
sleep(2);
var_dump(couchbase_get($handle, $key));

$key = "touch_test";
$value = "bar";
couchbase_add($handle, $key, $value);
var_dump(couchbase_get($handle, $key));
couchbase_touch($handle, $key, 3);
sleep(2);
var_dump(couchbase_get($handle, $key));
sleep(2);
var_dump(couchbase_get($handle, $key));
?>
--EXPECTF--
string(3) "foo"
NULL
string(3) "bar"
string(3) "bar"
NULL
