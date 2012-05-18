--TEST--
PCBC-67 Implement getResultMessage()
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$cb = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
$cb->get(uniqid());
var_dump($cb->getResultMessage());

$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);
couchbase_get($handle, uniqid());
var_dump(couchbase_get_result_message($handle));
?>
--EXPECTF--
string(11) "No such key"
string(11) "No such key"
