--TEST--
Check for couchbase_connect
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
var_dump($handle);
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, '80', COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
?>
--EXPECTF--
resource(%d) of type (Couchbase)

Warning: couchbase_connect(): Failed to connect libcouchbase to server: Protocol error in %s002.php on line %d
