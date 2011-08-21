--TEST--
Couchbase Set Get Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

var_dump(COUCHBASE_SUCCESS);
var_dump(COUCHBASE_AUTH_CONTINUE);

?>
--EXPECTF--
int(0)
int(1)
