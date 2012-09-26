--TEST--
Errors - ErrorCodes
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Errors", "testErrorCodes");
--EXPECT--
PHP_COUCHBASE_OK