--TEST--
Info - GetSetTimeout

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Info", "testGetSetTimeout");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Info", "testGetSetTimeout");
--EXPECT--
PHP_COUCHBASE_OK