--TEST--
Check for couchbase_get
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = uniqid("couchbase_value_");
$cas1 = couchbase_set($handle, $key, $value);

$v = couchbase_get($handle, $key, NULL, $cas2);
var_dump($v === $value);
var_dump($cas1 == $cas2);

couchbase_delete($handle, $key);
?>
--EXPECTF--
bool(true)
bool(true)
