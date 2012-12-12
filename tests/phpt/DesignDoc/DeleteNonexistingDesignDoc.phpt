--TEST--
DesignDoc - DeleteNonexistingDesignDoc

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testDeleteNonexistingDesignDoc");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testDeleteNonexistingDesignDoc");
--EXPECT--
PHP_COUCHBASE_OK