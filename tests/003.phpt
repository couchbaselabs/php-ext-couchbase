--TEST--
Check for couchbase_add
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";
var_dump(couchbase_add($handle, $key, $value));
couchbase_delete($handle, $key);

$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
var_dump($handle->add($key, $value));
$handle->delete($key);
?>
--EXPECTF--
string(%d) %s
string(%d) %s
