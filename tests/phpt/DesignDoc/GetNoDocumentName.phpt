--TEST--
DesignDoc - GetNoDocumentName

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testGetNoDocumentName");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testGetNoDocumentName");
--EXPECT--
PHP_COUCHBASE_OK