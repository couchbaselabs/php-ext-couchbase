--TEST--
EmptyKey - EmptyKeyGet
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("EmptyKey", "testEmptyKeyGet");
--EXPECT--
PHP_COUCHBASE_OK