--TEST--
Expiry - NegativeExpirySet

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Expiry", "testNegativeExpirySet");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testNegativeExpirySet");
--EXPECT--
PHP_COUCHBASE_OK