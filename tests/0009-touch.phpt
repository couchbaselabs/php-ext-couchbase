--TEST--
Couchbase Touch test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

function storage_callback($error, $key)
{
    if($error !== null) {
        var_dump($error);
    }
    var_dump($key);
}
couchbase_set_storage_callback($cb, "storage_callback");
couchbase_set($cb, "k", "v");

couchbase_execute($cb);

function touch_callback($error, $key)
{
    if($error !== null) {
        var_dump($error);
    }
    var_dump($key);
}

couchbase_set_touch_callback($cb, "touch_callback");
var_dump("set");
couchbase_mtouch($cb, "k", 10);

couchbase_execute($cb);

var_dump("end");
?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(1) "k"
string(3) "set"
string(3) "end"
