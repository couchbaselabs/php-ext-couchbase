--TEST--
Touch - TouchMultiNoKeysOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Touch", "testTouchMultiNoKeysOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Touch", "testTouchMultiNoKeysOO");
--EXPECT--
PHP_COUCHBASE_OK