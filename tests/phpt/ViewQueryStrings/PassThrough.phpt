--TEST--
ViewQueryStrings - PassThrough

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("ViewQueryStrings", "testPassThrough");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ViewQueryStrings", "testPassThrough");
--EXPECT--
PHP_COUCHBASE_OK