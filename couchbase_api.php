<?php
/*
  The ext/couchbase API


  @package Couchbase
*/

// functions
$couchbase_handle = couchbase_create($host, $user, $pass, $bucket);
$success = couchbase_connect($couchbase_handle);

$success = couchbase_set_get_callback($couchbase_handle, $callback);
$success = couchbase_set_storage_callback($couchbase_handle, $callback);
$success = couchbase_set_remove_callback($couchbase_handle, $callback);
$success = couchbase_set_arithmetic_callback($couchbase_handle, $callback);
$success = couchbase_set_touch_callback($couchbase_handle, $callback);

$success = couchbase_set_tap_mutation_callback($couchbase_handle, $callback);
$success = couchbase_set_tap_deletion_callback($couchbase_handle, $callback);
$success = couchbase_set_tap_opaque_callback($couchbase_handle, $callback);
$success = couchbase_set_tap_vbucket_set_callback($couchbase_handle, $callback);

$success = couchbase_get($couchbase_handle, $key_s, $expiry = null);
$success = couchbase_get_by_key($couchbase_handle, $hash, $key_s, $expiry = null);

$success = couchbase_set($couchbase_handle, $key, $value, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add($couchbase_handle, $key, $value, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace($couchbase_handle, $key, $value, $flags = null, $expiry = null, $cas = null);

$success = couchbase_set_by_key($couchbase_handle, $hash, $key, $value, $flags = null, $expiry = null, $cas = null);
$success = couchbase_add_by_key($couchbase_handle, $hash, $key, $value, $flags = null, $expiry = null, $cas = null);
$success = couchbase_replace_by_key($couchbase_handle, $hash, $key, $value, $flags = null, $expiry = null, $cas = null);

$success = couchbase_touch($couchbase_handle, $key_s, $expiry);
$success = couchbase_touch_by_key($couchbase_handle, $hash, $key_s, $expiry);

$success = couchbase_arithmetic($couchbase_handle, $key, $delta, $create, $initial); // useful for incr/decr etc
$success = couchbase_arithmetic_by_key($couchbase_handle, $hash, $key, $delta, $create, $initial); // useful for incr/decr etc

$success = couchbase_remove($couchbase_handle, $key, $cas = null);
$success = couchbase_remove_by_key($couchbase_handle, $hash, $key, $cas = null);

$success = couchbase_destroy($couchbase_handle);

couchbase_execute($couchbase_handle);
