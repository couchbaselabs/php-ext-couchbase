--TEST--
Delay - FetchOne
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Delay", "testFetchOne");
--EXPECT--
PHP_COUCHBASE_OK