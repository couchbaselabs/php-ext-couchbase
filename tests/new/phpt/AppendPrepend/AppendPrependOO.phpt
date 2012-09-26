--TEST--
AppendPrepend - AppendPrependOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("AppendPrepend", "testAppendPrependOO");
--EXPECT--
PHP_COUCHBASE_OK