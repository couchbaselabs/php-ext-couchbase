--TEST--
Couchbase Errors
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

// call function without callback
couchbase_set($cb, "k", "x");
couchbase_mget($cb, "k");
couchbase_remove($cb, "k");

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(3) "end"
