--TEST--
Check for couchbase_append/prepend
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";
couchbase_add($handle, $key, $value);

couchbase_prepend($handle, $key, "prefix_");
var_dump("prefix_" . $value === couchbase_get($handle, $key));

couchbase_append($handle, $key, "_suffix");
var_dump("prefix_" . $value . "_suffix" === couchbase_get($handle, $key));

couchbase_delete($handle, $key);
?>
--EXPECTF--
bool(true)
bool(true)
