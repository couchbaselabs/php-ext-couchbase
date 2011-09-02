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
    var_dump("> storage_callback(php)");
    //    if($error !== NULL) {
        var_dump($error);
        //    }
    var_dump($key);
    var_dump("< storage_callback(php)");
}

couchbase_set_storage_callback($cb, "storage_callback");

couchbase_set($cb, "k", "v");

couchbase_execute($cb);

// test too many parameter errors

//couchbase_set($cb, "k", "y", function($error, $key, $superfluous_arg) {});
//couchbase_mget($cb, "k", function($error, $key, $value, $superfluous_arg) {});

//couchbase_execute($cb);

var_dump("end");

?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)

string(3) "end"
