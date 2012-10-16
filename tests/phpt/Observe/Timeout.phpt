--TEST--
Observe - Timeout

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testTimeout");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testTimeout");
--EXPECT--
PHP_COUCHBASE_OK