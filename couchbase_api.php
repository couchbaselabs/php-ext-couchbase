<?php
/*
  The ext/couchbase API


  @package Couchbase
*/

// functions
$couchbase_handle = couchbase_create($host, $user, $pass, $bucket);
$success = couchbase_connect($couchbase_handle);

$success = couchbase_get($couchbase_handle, $key_s, $callback, $expiry = null);
$success = couchbase_get_by_key($couchbase_handle, $hash, $key_s, $callback, $expiry = null);

$success = couchbase_set($couchbase_handle, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add($couchbase_handle, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace($couchbase_handle, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);

$success = couchbase_set_by_key($couchbase_handle, $hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add_by_key($couchbase_handle, $hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace_by_key($couchbase_handle, $hash, $key, $value, $callback, $flags = null, $expiry = null, $cas = null);

$success = couchbase_touch($couchbase_handle, $key_s, $expiry, $callback);
$success = couchbase_touch_by_key($couchbase_handle, $hash, $key_s, $expiry, $callback);

$success = couchbase_arithmetic($couchbase_handle, $key, $delta, $create, $initial, $callback); // useful for incr/decr etc
$success = couchbase_arithmetic_by_key($couchbase_handle, $hash, $key, $delta, $create, $initial, $callback); // useful for incr/decr etc

$success = couchbase_remove($couchbase_handle, $key, $callback, $cas = null);
$success = couchbase_remove_by_key($couchbase_handle, $hash, $key, $callback, $cas = null);

$success = couchbase_destroy($couchbase_handle);

couchbase_execute($couchbase_handle);
