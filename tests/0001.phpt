--TEST--
Couchbase Basic Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php
couchbase_hello();
?>
--EXPECT--
Hello Couchbase