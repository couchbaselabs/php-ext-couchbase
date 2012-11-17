--TEST--
ViewQueryStrings - MiscNumeric

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewQueryStrings", "testMiscNumeric");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewQueryStrings", "testMiscNumeric");
--EXPECT--
PHP_COUCHBASE_OK