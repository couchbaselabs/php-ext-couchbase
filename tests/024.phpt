--TEST--
Check for get/set_option
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

var_dump(couchbase_set_option($handle, COUCHBASE_OPT_SERIALIZER, COUCHBASE_SERIALIZER_PHP));
var_dump(couchbase_get_option($handle, COUCHBASE_OPT_SERIALIZER) == COUCHBASE_SERIALIZER_PHP);
var_dump(couchbase_set_option($handle, COUCHBASE_OPT_SERIALIZER, 1111));
var_dump(couchbase_set_option($handle, 1111, 1111));
?>
--EXPECTF--
bool(true)
bool(true)

Warning: couchbase_set_option(): Unsupported serializer:1111 in %s024.php on line %d
bool(false)

Warning: couchbase_set_option(): Unknow option type:1111 in %s024.php on line %d
bool(false)
