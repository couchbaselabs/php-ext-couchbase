--TEST--
Check for couchbase_cas
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$cas = couchbase_add($handle, $key, "foo");
$ncas = couchbase_set($handle, $key, "bar");

var_dump(couchbase_cas($handle, $cas, $key, "dummy"));
var_dump(couchbase_cas($handle, $ncas, $key, "dummy", 1));
var_dump(couchbase_get($handle, $key, NULL, $cas1));
sleep(2);
var_dump(couchbase_get($handle, $key));

couchbase_delete($handle, $key);
?>
--EXPECTF--
bool(false)
bool(true)
string(5) "dummy"
NULL
