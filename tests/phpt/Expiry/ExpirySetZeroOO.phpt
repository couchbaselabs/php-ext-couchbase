--TEST--
Expiry - ExpirySetZeroOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Expiry", "testExpirySetZeroOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testExpirySetZeroOO");
--EXPECT--
PHP_COUCHBASE_OK