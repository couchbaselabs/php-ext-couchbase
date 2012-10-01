--TEST--
NegativeExpiry - Append
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testAppend");
--EXPECT--
PHP_COUCHBASE_OK