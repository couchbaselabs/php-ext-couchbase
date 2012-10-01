--TEST--
NegativeExpiry - SetMulti
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testSetMulti");
--EXPECT--
PHP_COUCHBASE_OK