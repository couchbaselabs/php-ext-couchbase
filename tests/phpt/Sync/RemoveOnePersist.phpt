--TEST--
Sync - RemoveOnePersist

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Sync", "testRemoveOnePersist");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Sync", "testRemoveOnePersist");
--EXPECT--
PHP_COUCHBASE_OK