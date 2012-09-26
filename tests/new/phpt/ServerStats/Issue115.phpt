--TEST--
ServerStats - Issue115
--FILE--
<?php
include dirname(__FILE__)."/../../cbtestframework/cbtest-phpt-loader.inc";
couchbase_phpt_runtest("ServerStats", "testIssue115");
--EXPECT--
PHP_COUCHBASE_OK