--TEST--
Info - GetSetTimeoutOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Info", "testGetSetTimeoutOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Info", "testGetSetTimeoutOO");
--EXPECT--
PHP_COUCHBASE_OK