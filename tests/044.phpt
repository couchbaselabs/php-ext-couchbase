--TEST--
PCBC-117, checks for negative expiry in increment operation
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
couchbase_add($handle, $key, 42);
var_dump(couchbase_increment($handle, $key, 1, false, -1));
?>
--EXPECTF--
Fatal error: Expiry must not be negative (%i given). in %s
