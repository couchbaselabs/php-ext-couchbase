--TEST--
ViewQueryStrings - EncodeStringToJson

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewQueryStrings", "testEncodeStringToJson");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewQueryStrings", "testEncodeStringToJson");
--EXPECT--
PHP_COUCHBASE_OK