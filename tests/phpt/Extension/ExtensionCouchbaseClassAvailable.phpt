--TEST--
Extension - ExtensionCouchbaseClassAvailable

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Extension", "testExtensionCouchbaseClassAvailable");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Extension", "testExtensionCouchbaseClassAvailable");
--EXPECT--
PHP_COUCHBASE_OK