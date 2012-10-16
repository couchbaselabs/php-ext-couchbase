--TEST--
Observe - PollingMultiMixed

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testPollingMultiMixed");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testPollingMultiMixed");
--EXPECT--
PHP_COUCHBASE_OK