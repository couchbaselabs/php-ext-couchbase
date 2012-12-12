--TEST--
Touch - Touch

--SKIPIF--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_skipif("Touch", "testTouch");

--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("Touch", "testTouch");
--EXPECT--
PHP_COUCHBASE_OK