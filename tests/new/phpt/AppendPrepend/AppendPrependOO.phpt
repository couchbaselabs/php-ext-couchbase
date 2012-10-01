--TEST--
AppendPrepend - AppendPrependOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("AppendPrepend", "testAppendPrependOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendPrependOO");
--EXPECT--
PHP_COUCHBASE_OK