--TEST--
Check for couchbase_set
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
var_dump($cas1 = couchbase_set($handle, $key, uniqid("couchbase_value_")));
var_dump($cas2 = couchbase_set($handle, $key, uniqid("couchbase_value_")));

var_dump($cas1 != $cas2);

couchbase_delete($handle, $key);
?>
--EXPECTF--
float(%s)
float(%s)
bool(true)
