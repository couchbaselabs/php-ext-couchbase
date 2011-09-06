--TEST--
Couchbase Add test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

function storage_callback($error, $key)
{
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
}

couchbase_set_storage_callback($cb, "storage_callback");

couchbase_set($cb, "k", "x");

couchbase_execute($cb);

function storage_callback2($error, $key)
{
    var_dump($error == COUCHBASE_KEY_EEXISTS);
    var_dump($key);
}

couchbase_set_storage_callback($cb, "storage_callback2");

couchbase_add($cb, "k", "y");

function remove_callback($error, $key)
{
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
}

couchbase_set_remove_callback($cb, "remove_callback");

couchbase_remove($cb, "l");

couchbase_execute($cb);

couchbase_set_storage_callback($cb, "storage_callback");

couchbase_add($cb, "l", "z");

couchbase_execute($cb);

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
bool(true)
string(1) "k"
string(1) "l"
string(1) "l"
string(3) "end"
