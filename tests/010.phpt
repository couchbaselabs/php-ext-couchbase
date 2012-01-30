--TEST--
Check for couchbase_increment/couchbase_decrement
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$key = uniqid("couchbase_");
$value = "2";
couchbase_add($handle, $key, $value);
var_dump(couchbase_increment($handle, $key));
var_dump(couchbase_get($handle, $key));
var_dump(couchbase_decrement($handle, $key, 2));
var_dump(couchbase_get($handle, $key));

couchbase_set($handle, $key, "string");
var_dump(couchbase_increment($handle, $key));

couchbase_delete($handle, $key);

// test incrementing a nonexisting key (PCBC-30)
var_dump(couchbase_increment($handle, "key", $offset = 1, $create = true, $expire = NULL, $initial_value = 2));
var_dump(couchbase_get($handle, "key"));
couchbase_delete($handle, "key");

?>
--EXPECTF--
int(3)
string(1) "3"
int(1)
string(1) "1"

Warning: couchbase_increment(): Faild to increment value in server: Not a number in %s010.php on line %d
bool(false)
int(2)
int(2)
