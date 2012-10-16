--TEST--
Info - NumReplicas

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Info", "testNumReplicas");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Info", "testNumReplicas");
--EXPECT--
PHP_COUCHBASE_OK