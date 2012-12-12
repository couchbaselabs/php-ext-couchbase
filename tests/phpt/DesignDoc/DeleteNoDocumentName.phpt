--TEST--
DesignDoc - DeleteNoDocumentName

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testDeleteNoDocumentName");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testDeleteNoDocumentName");
--EXPECT--
PHP_COUCHBASE_OK