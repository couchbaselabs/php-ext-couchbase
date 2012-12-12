--TEST--
DesignDoc - GetSetDeleteDesignDocOO

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testGetSetDeleteDesignDocOO");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testGetSetDeleteDesignDocOO");
--EXPECT--
PHP_COUCHBASE_OK