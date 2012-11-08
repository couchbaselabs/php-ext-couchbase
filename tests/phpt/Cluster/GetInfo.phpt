--TEST--
Cluster - GetInfo

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Cluster", "testGetInfo");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Cluster", "testGetInfo");
--EXPECT--
PHP_COUCHBASE_OK