--TEST--
Check for couchbase_flush
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$keys = array();
$cas  = array();

$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");

foreach ($keys as $k => $v) {
   $cas[$k] = couchbase_set($handle, $k, $v);
}

$v = couchbase_get_multi($handle, array_keys($keys), $cas1);

var_dump(serialize($v) == serialize($keys));
var_dump(serialize($cas) == serialize($cas1));

couchbase_flush($handle);
var_dump(count(couchbase_get_multi($handle, array_keys($keys))));

$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$cas2 = array();
foreach ($keys as $k => $v) {
   $cas2[$k] = $handle->set($k, $v);
}

$v = $handle->getMulti(array_keys($keys), $cas3);

var_dump(serialize($v) == serialize($keys));
var_dump(serialize($cas2) == serialize($cas3));

$handle->flush();
var_dump(count($handle->getMulti(array_keys($keys))));
?>
--EXPECTF--
bool(true)
bool(true)
int(0)
bool(true)
bool(true)
int(0)
