--TEST--
Unlock - Unlock

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Unlock", "testUnlock");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Unlock", "testUnlock");
--EXPECT--
PHP_COUCHBASE_OK