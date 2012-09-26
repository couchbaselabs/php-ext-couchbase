--TEST--
Expiry - ArithmeticExpiry
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testArithmeticExpiry");
--EXPECT--
PHP_COUCHBASE_OK