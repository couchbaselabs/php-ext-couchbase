--TEST--
Couchbase Create Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php
$host = "localhost:9000";
$cb = couchbase_create("$host");
var_dump($cb);

$cb = couchbase_create("$host", "user");
var_dump($cb);

$cb = couchbase_create("$host", "user", "pass");
var_dump($cb);

$cb = couchbase_create("$host", "user", "pass", "bucket");
var_dump($cb);
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
resource(%d) of type (Couchbase Instance)
resource(%d) of type (Couchbase Instance)
resource(%d) of type (Couchbase Instance)
