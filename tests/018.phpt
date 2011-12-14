--TEST--
Check for empty key
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--
precision=19
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

var_dump(couchbase_set($handle, "", "foo"));
var_dump(couchbase_get($handle, "", "foo"));
var_dump(couchbase_replace($handle, "", "foo"));
var_dump(couchbase_add($handle, "", "foo"));
print_r(couchbase_get_multi($handle, array("")));
print_r(couchbase_set_multi($handle, array("" => 2)));
?>
--EXPECTF--
Warning: couchbase_set(): Failed to schedule set request: Empty key in %s018.php on line %d
bool(false)
NULL

Warning: couchbase_replace(): Failed to schedule set request: Empty key in %s018.php on line %d
bool(false)

Warning: couchbase_add(): Failed to schedule set request: Empty key in %s018.php on line %d
bool(false)
Array
(
)
Array
(
)
