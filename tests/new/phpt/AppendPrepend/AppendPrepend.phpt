--TEST--
AppendPrepend - AppendPrepend
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendPrepend");
--EXPECT--
PHP_COUCHBASE_OK