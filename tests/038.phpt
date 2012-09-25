--TEST--
PCBC-117, checks for negative expiry in set operation
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";

var_dump(couchbase_set($handle, $key, $value, -1));
?>
--EXPECTF--
Fatal error: Expiry must not be negative (%i given). in %s
