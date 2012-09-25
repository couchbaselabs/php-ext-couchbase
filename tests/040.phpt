--TEST--
PCBC-117, checks for negative expiry in replace operation
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
couchbase_add($handle, $key, "foo");
var_dump(couchbase_replace($handle, $key, "bar", -1));
?>
--EXPECTF--
Fatal error: Expiry must not be negative (%i given). in %s
