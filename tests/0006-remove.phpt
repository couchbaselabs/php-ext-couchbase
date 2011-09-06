--TEST--
Couchbase Remove test
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

function get_callback($error, $key, $value)
{
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
    var_dump($value);
}

couchbase_set_get_callback($cb, "get_callback");

couchbase_mget($cb, "k");

couchbase_execute($cb);

function remove_callback($error, $key)
{
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
}

couchbase_set_remove_callback($cb, "remove_callback");

couchbase_remove($cb, "k");

couchbase_execute($cb);

function get_callback2($error, $key, $value) {
    var_dump($error === COUCHBASE_KEY_ENOENT);
    var_dump($value);
}

couchbase_set_get_callback($cb, "get_callback2");

couchbase_mget($cb, "k");

couchbase_execute($cb);

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
string(1) "k"
string(1) "x"
string(1) "k"
bool(true)
NULL
string(3) "end"
