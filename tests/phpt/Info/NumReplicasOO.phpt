--TEST--
Info - NumReplicasOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Info", "testNumReplicasOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Info", "testNumReplicasOO");
--EXPECT--
PHP_COUCHBASE_OK