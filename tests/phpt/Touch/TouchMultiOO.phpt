--TEST--
Touch - TouchMultiOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Touch", "testTouchMultiOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Touch", "testTouchMultiOO");
--EXPECT--
PHP_COUCHBASE_OK