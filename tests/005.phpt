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

function cache_cb($res, $key, &$value) {
      $value = "from db";
      return true;
}

couchbase_get($handle, $key, "cache_cb", $cas3);
var_dump($cas3 === $cas2);

var_dump("from db" === couchbase_get($handle, "non-exists-key", "cache_cb", $cas4));
var_dump($cas4);

function another_cache_cb($res, $key, &$value) {
      $value = "from db";
      return false;
}

var_dump(NULL === couchbase_get($handle, "non-exists-key", "another_cache_cb"));

couchbase_delete($handle, $key);

$newkey = uniqid("couchbase_locked_");
$newval = uniqid("couchbase_value_");
$cas5 = couchbase_set($handle, $newkey, $newval);
var_dump($newval === couchbase_get($handle, $newkey, NULL, $cas6, 1, true)); // get-with-lock
var_dump($cas5 != $cas6); // locking should mutate, hence changed CAS
sleep(2);
couchbase_delete($handle, $newkey);

/* OO APIs */
$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$cas1 = $handle->set($key, $value);

$v = $handle->get($key, NULL, $cas2);
var_dump($v === $value);
var_dump($cas1 == $cas2);

$handle->get($key, "cache_cb", $cas3);
var_dump($cas3 === $cas2);

var_dump("from db" === $handle->get("non-exists-key", "cache_cb", $cas4));
var_dump($cas4);

var_dump(NULL === $handle->get("non-exists-key", "another_cache_cb"));

$handle->delete($key);

$cas5 = $handle->set($newkey, $newval);
var_dump($newval === $handle->get($newkey, NULL, $cas6, 1, true)); // get-with-lock
var_dump($cas5 != $cas6); // locking should mutate, hence changed CAS
sleep(2);

$handle->delete($newkey);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
NULL
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
NULL
bool(true)
bool(true)
bool(true)
