--TEST--
Check for couchbase_get_stats
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$stats = couchbase_get_stats($handle);
var_dump(is_array($stats));
var_dump(count($stats) > 0);
var_dump(count(current($stats)) > 10);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
