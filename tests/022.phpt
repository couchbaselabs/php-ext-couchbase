--TEST--
Check for couchbase_increment/couchbase_decrement rest args
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");

var_dump(couchbase_increment($handle, $key, 2, 1, 0, 2));
couchbase_delete($handle, $key);
var_dump(couchbase_decrement($handle, $key, 2, 1, 0));
couchbase_delete($handle, $key);
var_dump(couchbase_decrement($handle, $key, 2));

couchbase_delete($handle, $key);
?>
--EXPECTF--
int(2)
int(0)
bool(false)