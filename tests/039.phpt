--TEST--
PCBC-117, checks for negative expiry in set-multi operation
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key1 = uniqid("couchbase_");
$key2 = uniqid("couchbase_");
$value = "foo";
$kv_array = array($key1 => $value, $key2 => $value);

var_dump(couchbase_set_multi($handle, $kv_array, -1));
?>
--EXPECTF--
Fatal error: Expiry must not be negative (%i given). in %s
