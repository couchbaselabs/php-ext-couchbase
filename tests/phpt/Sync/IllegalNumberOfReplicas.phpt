--TEST--
Sync - IllegalNumberOfReplicas

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Sync", "testIllegalNumberOfReplicas");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Sync", "testIllegalNumberOfReplicas");
--EXPECT--
PHP_COUCHBASE_OK