--TEST--
Check for php serializer
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");

$value = array(1, 2, 3, "test" => "bar", array("dummy"));
$value2 = new stdClass();
$value2->test =  "bar";
$value2->arr  = array("dummy");
$value3 = NULL;
$value4 = TRUE;

couchbase_add($handle, $key, $value);
var_dump(couchbase_get($handle, $key) === $value);
couchbase_set($handle, $key, $value2);
var_dump(couchbase_get($handle, $key) == $value2);
couchbase_set($handle, $key, $value3);
var_dump(couchbase_get($handle, $key) === $value3);
couchbase_set($handle, $key, $value4);
var_dump(couchbase_get($handle, $key) === $value4);

$fp = fopen(__FILE__, "r");
couchbase_set($handle, $key, $fp);
var_dump(couchbase_get($handle, $key));

couchbase_delete($handle, $key);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
int(%d)
