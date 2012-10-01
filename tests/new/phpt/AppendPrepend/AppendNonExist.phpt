--TEST--
AppendPrepend - AppendNonExist

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("AppendPrepend", "testAppendNonExist");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendNonExist");
--EXPECT--
PHP_COUCHBASE_OK