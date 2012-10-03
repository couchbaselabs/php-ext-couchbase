--TEST--
EmptyKey - EmptyKeyReplace

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("EmptyKey", "testEmptyKeyReplace");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("EmptyKey", "testEmptyKeyReplace");
--EXPECT--
PHP_COUCHBASE_OK