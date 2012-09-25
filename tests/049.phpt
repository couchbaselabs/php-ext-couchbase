--TEST--
PCBC-117, checks for negative expiry in touch operation
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "foo";

couchbase_add($handle, $key, $value);

var_dump(couchbase_touch($handle, $key, -1));
?>
--EXPECTF--
Fatal error: Expiry must not be negative (%i given). in %s
