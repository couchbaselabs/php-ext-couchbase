--TEST--
PCBC-67 preserve order option for getMulti
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$cb = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$data = array();
foreach(range(0, 9) AS $i) {
	$data[uniqid($i . "_")] = uniqid($i . "_");
}

asort($data);

foreach($data AS $key => $val) {
	$cb->set($key, $val);
}

$keys = array_keys($data);
$resultA = $cb->getMulti($keys);

$cas = array();
$resultB = $cb->getMulti($keys, $cas, Couchbase::GET_PRESERVE_ORDER);
$errors = false;

// this only ever fails with more than one node
if(serialize($data) != serialize($resultB)) {
	$errors = true;
	var_dump($data);
	var_dump($resultB);
}

var_dump($errors)
?>
--EXPECTF--
bool(false)
