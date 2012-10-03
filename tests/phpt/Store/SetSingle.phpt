--TEST--
Store - SetSingle

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Store", "testSetSingle");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Store", "testSetSingle");
--EXPECT--
PHP_COUCHBASE_OK