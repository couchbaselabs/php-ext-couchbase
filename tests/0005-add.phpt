--TEST--
Couchbase Add test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

couchbase_set($cb, "k", "x", function($error, $key) {
    var_dump($key);
});

couchbase_add($cb, "k", "y", function($error, $key) {
    var_dump($key);
    var_dump($error == COUCHBASE_KEY_EEXISTS);
});

couchbase_remove($cb, "l", function($error, $key) {
    var_dump($key);
});

couchbase_execute($cb);

couchbase_add($cb, "l", "z", function($error, $key) {
    var_dump($key);
    var_dump($error);
});

couchbase_execute($cb);

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
string(1) "k"
bool(true)
string(1) "l"
string(1) "l"
NULL
string(3) "end"
