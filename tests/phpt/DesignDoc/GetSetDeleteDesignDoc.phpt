--TEST--
DesignDoc - GetSetDeleteDesignDoc

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("DesignDoc", "testGetSetDeleteDesignDoc");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("DesignDoc", "testGetSetDeleteDesignDoc");
--EXPECT--
PHP_COUCHBASE_OK