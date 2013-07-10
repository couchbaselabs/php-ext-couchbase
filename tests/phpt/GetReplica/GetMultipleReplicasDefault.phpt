--TEST--
GetReplica - GetMultipleReplicasDefault

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("GetReplica", "testGetMultipleReplicasDefault");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("GetReplica", "testGetMultipleReplicasDefault");
--EXPECT--
PHP_COUCHBASE_OK