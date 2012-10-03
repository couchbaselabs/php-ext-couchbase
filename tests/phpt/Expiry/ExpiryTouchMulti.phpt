--TEST--
Expiry - ExpiryTouchMulti

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Expiry", "testExpiryTouchMulti");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Expiry", "testExpiryTouchMulti");
--EXPECT--
PHP_COUCHBASE_OK