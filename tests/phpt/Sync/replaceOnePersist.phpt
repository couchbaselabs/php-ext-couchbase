--TEST--
Sync - ReplaceOnePersist

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Sync", "testReplaceOnePersist");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Sync", "testReplaceOnePersist");
--EXPECT--
PHP_COUCHBASE_OK