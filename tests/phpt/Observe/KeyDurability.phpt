--TEST--
Observe - KeyDurability

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Observe", "testKeyDurability");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Observe", "testKeyDurability");
--EXPECT--
PHP_COUCHBASE_OK