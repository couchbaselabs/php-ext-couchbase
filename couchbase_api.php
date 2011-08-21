<?php
/*
  The ext/couchbase API


  @package Couchbase
*/

// functions
$couchbase_handle = couchbase_create($host, $user, $pass, $bucket);
$success = couchbase_connect($couchbase_handle);

$success = couchbase_get($key_s, $callback, $expiry = null);
$success = couchbase_get_by_key($hash, $key_s, $callback, $expiry = null);

$success = couchbase_set($key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add($key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace($key, $value, $callback, $flags = null, $expiry = null, $cas = null);

$success = couchbase_set_by_key($hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add_by_key($hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace_by_key($hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);

$success = couchbase_touch($key_s, $expiry, $callback);
$success = couchbase_touch_by_key($hash, $key_s, $expiry, $callback);

$success = couchbase_arithmetic($key, $delta, $create, $initial, $callback); // useful for incr/decr etc
$success = couchbase_arithmetic_by_key($hash, $key, $delta, $create, $initial, $callback); // useful for incr/decr etc

$success = couchbase_remove($key, $callback, $cas = null);
$success = couchbase_remove_by_key($hash, $key, $callback, $cas = null);

$success = couchbase_destroy($couchbase_handle);

couchbase_execute($couchbase_handle);
