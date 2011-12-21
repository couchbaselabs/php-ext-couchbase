--TEST--
Check for couchbase_delete
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";

couchbase_add($handle, $key, $value);
var_dump(couchbase_get($handle, $key));
couchbase_delete($handle, $key);
var_dump(couchbase_get($handle, $key));


$cas = couchbase_add($handle, $key, $value);
var_dump(couchbase_get($handle, $key));

$cas2 = couchbase_set($handle, $key, "bar");

var_dump(couchbase_delete($handle, $key, $cas));
var_dump(couchbase_get($handle, $key));

var_dump(couchbase_delete($handle, $key, $cas2));
var_dump(couchbase_get($handle, $key));
?>
--EXPECTF--
string(3) "foo"
NULL
string(3) "foo"
bool(false)
string(3) "bar"
bool(true)
NULL
