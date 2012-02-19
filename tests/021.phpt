--TEST--
Check for couchbase delayed & fetch one
--SKIPIF--
<?php include "skipif.inc" ?>
--INI--

--FILE--
<?php
include "couchbase.inc";
$handle = couchbase_connect(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

$keys = array();
$values = array();

$values[uniqid("couchbase_")] = uniqid("couchbase_value_");
$values[uniqid("couchbase_")] = uniqid("couchbase_value_");
$values[uniqid("couchbase_")] = uniqid("couchbase_value_");
$values[uniqid("couchbase_")] = uniqid("couchbase_value_");

$keys = array_flip(array_keys($values));

couchbase_set_multi($handle, $values);

var_dump(couchbase_get_delayed($handle, array_keys($values), false));
while ($row = couchbase_fetch($handle)) {
    if (!in_array($row["value"], $values)) {
        die("error");
    }
    $keys[$row["key"]] = -1;
}

foreach($keys as $v) {
   if ($v !== -1) {
        die("error");
   }
}

var_dump(couchbase_fetch($handle));
$values[uniqid("couchbase_")] = uniqid("couchbase_value_");
$keys = array_flip(array_keys($values));

couchbase_set_multi($handle, $values);
var_dump(couchbase_get_delayed($handle, array_keys($values), false));
while ($row = couchbase_fetch($handle)) {
    if (!in_array($row["value"], $values)) {
        die("error");
    }
    $keys[$row["key"]] = -1;
}

foreach($keys as $v) {
   if ($v !== -1) {
        die("error");
   }
}

couchbase_flush($handle);
--EXPECTF--
bool(true)
bool(false)
bool(true)
