--TEST--
Sync - PrependOnePersist

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Sync", "testPrependOnePersist");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Sync", "testPrependOnePersist");
--EXPECT--
PHP_COUCHBASE_OK