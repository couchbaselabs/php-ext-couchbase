--TEST--
Couchbase Basic Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php
echo couchbase_version();
?>
--EXPECT--
0.0.1