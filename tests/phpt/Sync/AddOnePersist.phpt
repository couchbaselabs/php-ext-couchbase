--TEST--
Sync - AddOnePersist

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Sync", "testAddOnePersist");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Sync", "testAddOnePersist");
--EXPECT--
PHP_COUCHBASE_OK