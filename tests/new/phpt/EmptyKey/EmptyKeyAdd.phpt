--TEST--
EmptyKey - EmptyKeyAdd
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("EmptyKey", "testEmptyKeyAdd");
--EXPECT--
PHP_COUCHBASE_OK