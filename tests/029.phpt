--TEST--
PCBC-54 Check for storing integer values
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$cb = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$cb->set("userid", 642349292);
$cb->set("productid", 5);
var_dump($cb->get("userid"));
var_dump($cb->get("productid"));
?>
--EXPECTF--
int(642349292)
int(5)
