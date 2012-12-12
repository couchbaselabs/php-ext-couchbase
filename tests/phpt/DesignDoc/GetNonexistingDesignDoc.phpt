--TEST--
DesignDoc - GetNonexistingDesignDoc

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testGetNonexistingDesignDoc");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testGetNonexistingDesignDoc");
--EXPECT--
PHP_COUCHBASE_OK