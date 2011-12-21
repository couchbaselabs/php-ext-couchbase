--TEST--
Check for couchbase_replace
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";
var_dump($cas = couchbase_add($handle, $key, "dummy"));
var_dump(couchbase_replace($handle, $key, $value));
var_dump(couchbase_get($handle, $key));

couchbase_delete($handle, $key);
var_dump(couchbase_replace($handle, $key, $value));

var_dump(COUCHBASE_KEY_ENOENT == couchbase_get_result_code($handle));
?>
--EXPECTF--
string(%d) %s
string(%d) %s
string(3) "foo"
bool(false)
bool(true)
