--TEST--
Couchbase Replace test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

function storage_callback($error, $key)
{
    var_dump($error == COUCHBASE_KEY_ENOENT);
    var_dump($key);
}

couchbase_set_storage_callback($cb, "storage_callback");

couchbase_replace($cb, "k", "y");

couchbase_execute($cb);

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
bool(true)
string(1) "k"
string(3) "end"
