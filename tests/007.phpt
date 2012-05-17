--TEST--
Check for couchbase_get_multi
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

function do_test()
{
	global $handle;
	$keys = array();
	$cas  = array();

	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
	$keys["a"] = uniqid("couchbase_value_");
	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");
	$keys[uniqid("couchbase_")] = uniqid("couchbase_value_");

	foreach ($keys as $k => $v) {
	   $cas[$k] = couchbase_set($handle, $k, $v);
	}

	$v = couchbase_get_multi($handle, array_keys($keys), $cas1);

	var_dump(serialize($v) == serialize($keys));
	var_dump(serialize($cas) == serialize($cas1));

	$k = array_keys($keys);
	var_dump($v[$k[0]] == $keys[$k[0]]);
}

do_test();
couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, "foo_");
do_test();
couchbase_set_option($handle, COUCHBASE_OPT_PREFIX_KEY, "");

couchbase_flush($handle);
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
