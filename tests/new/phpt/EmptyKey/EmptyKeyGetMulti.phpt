--TEST--
EmptyKey - EmptyKeyGetMulti
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("EmptyKey", "testEmptyKeyGetMulti");
--EXPECT--
PHP_COUCHBASE_OK