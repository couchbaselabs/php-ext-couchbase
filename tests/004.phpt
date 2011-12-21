--TEST--
Check for couchbase_set
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
var_dump($cas1 = couchbase_set($handle, $key, "bar"));
var_dump($cas2 = couchbase_set($handle, $key, "bar"));

var_dump($cas1 != $cas2);

var_dump(couchbase_set($handle, $key, "foo", 0, $cas1));
var_dump(couchbase_get($handle, $key));
var_dump(couchbase_set($handle, $key, "foo", 0, $cas2));
var_dump(couchbase_get($handle, $key));

couchbase_delete($handle, $key);
?>
--EXPECTF--
string(%d) %s
string(%d) %s
bool(true)
bool(false)
string(3) "bar"
string(%d) %s
string(3) "foo"
