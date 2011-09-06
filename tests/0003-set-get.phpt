--TEST--
Couchbase Set Get Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

// test global function callbacks

function storage_callback($error, $key) {
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
}

couchbase_set_storage_callback($cb, "storage_callback");

function get_callback($error, $key, $value) {
    if($error !== NULL) {
        var_dump($error);
    }
    var_dump($key);
    var_dump($value);
}

couchbase_set_get_callback($cb, "get_callback");

couchbase_set($cb, "k", "v");
couchbase_mget($cb, "k");
couchbase_execute($cb);

// test too many parameter errors

couchbase_set($cb, "k", "y", function($error, $key, $superfluous_arg) {});
couchbase_mget($cb, "k", function($error, $key, $value, $superfluous_arg) {});

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
string(1) "k"
string(1) "v"

Warning: couchbase_set() expects exactly 3 parameters, 4 given in %s on line %d

Warning: couchbase_mget() expects exactly 2 parameters, 3 given in %s on line %d
string(3) "end"