--TEST--
Couchbase Remove test
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

couchbase_mget($cb, "k", function($error, $key, $value) {
    var_dump($value);
});

couchbase_execute($cb);

couchbase_remove($cb, "k", function($error, $key) {
    var_dump($key);
    var_dump($error);
});

couchbase_execute($cb);

couchbase_mget($cb, "k", function($error, $key, $value) {
    var_dump($error === COUCHBASE_KEY_ENOENT);
    var_dump($value);
});

couchbase_execute($cb);


var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
string(1) "x"
string(1) "k"
NULL
bool(true)
NULL
string(3) "end"
