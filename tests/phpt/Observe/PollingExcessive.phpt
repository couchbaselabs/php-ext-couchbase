--TEST--
Observe - PollingExcessive

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testPollingExcessive");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testPollingExcessive");
--EXPECT--
PHP_COUCHBASE_OK