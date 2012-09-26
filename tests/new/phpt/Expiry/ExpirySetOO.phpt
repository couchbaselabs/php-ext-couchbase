--TEST--
Expiry - ExpirySetOO
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testExpirySetOO");
--EXPECT--
PHP_COUCHBASE_OK