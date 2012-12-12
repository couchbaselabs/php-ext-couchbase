--TEST--
Touch - TouchNoKeyOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Touch", "testTouchNoKeyOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Touch", "testTouchNoKeyOO");
--EXPECT--
PHP_COUCHBASE_OK