--TEST--
Unlock - UnlockOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Unlock", "testUnlockOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Unlock", "testUnlockOO");
--EXPECT--
PHP_COUCHBASE_OK