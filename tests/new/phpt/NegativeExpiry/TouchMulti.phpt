--TEST--
NegativeExpiry - TouchMulti
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("NegativeExpiry", "testTouchMulti");
--EXPECT--
PHP_COUCHBASE_OK