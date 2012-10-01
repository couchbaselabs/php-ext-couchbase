--TEST--
Expiry - ExpiryTouch

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Expiry", "testExpiryTouch");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testExpiryTouch");
--EXPECT--
PHP_COUCHBASE_OK