--TEST--
Cluster - InvalidParams

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Cluster", "testInvalidParams");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Cluster", "testInvalidParams");
--EXPECT--
PHP_COUCHBASE_OK