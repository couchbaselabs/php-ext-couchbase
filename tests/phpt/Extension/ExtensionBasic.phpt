--TEST--
Extension - ExtensionBasic

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Extension", "testExtensionBasic");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Extension", "testExtensionBasic");
--EXPECT--
PHP_COUCHBASE_OK