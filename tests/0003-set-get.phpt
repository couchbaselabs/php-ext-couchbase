--TEST--
Couchbase Set Get Test
--SKIPIF--
<?php if (!extension_loaded("couchbase")) echo 'skip'; ?>
--FILE--
<?php

$host = "localhost:9000";
$cb = couchbase_create($host, "Administrator", "asdasd", "default");
var_dump($cb);

// test global function callbacks
function set_callback($error) {
  if($error !== NULL) {
    var_dump("nooo");
    var_dump($error);
  }
  var_dump("yay");
}

function get_callback($error, $key, $value) {
    if($error !== NULL) {
      var_dump($error);
    }
    var_dump($key);
    var_dump($value);
}

couchbase_set($cb, "k", "x", "set_callback");
couchbase_mget($cb, "k", "get_callback");

couchbase_execute($cb);

// test closure callbacks

couchbase_set($cb, "k", "y", function($error) {
  if($error !== NULL) {
    var_dump("nooo");
    var_dump($error);
  }
  var_dump("yay");
});

couchbase_mget($cb, "k", function($error, $key, $value) {
    if($error !== NULL) {
      var_dump($error);
    }
    var_dump($key);
    var_dump($value);
});

couchbase_execute($cb);

// test class / static method callbacks

class Callbacks
{
    static function set_callback($error) {
        if($error !== NULL) {
            var_dump("nooo");
            var_dump($error);
        }
        var_dump("yay");
    }

    static function get_callback($error, $key, $value) {
        if($error !== NULL) {
            var_dump($error);
        }
        var_dump($key);
        var_dump($value);
    }
}

couchbase_set($cb, "k", "z", array("Callbacks", "set_callback"));
couchbase_mget($cb, "k", array("Callbacks", "get_callback"));

// test object / method callbacks
class Callbacks2
{
    function set_callback($error) {
        if($error !== NULL) {
            var_dump("nooo");
            var_dump($error);
        }
        var_dump("yay");
    }

    function get_callback($error, $key, $value) {
        if($error !== NULL) {
            var_dump($error);
        }
        var_dump($key);
        var_dump($value);
    }
}

$cbs = new Callbacks2;
couchbase_set($cb, "k", "zz", array($cbs, "set_callback"));
couchbase_mget($cb, "k", array($cbs, "get_callback"));

// test too many parameter errors

couchbase_set($cb, "k", "y", function($error, $superfluous_arg) {});
couchbase_mget($cb, "k", function($error, $key, $value, $superfluous_arg) {});

couchbase_execute($cb);

var_dump("end");

?>
--EXPECTF--
resource(%d) of type (Couchbase Instance)
string(3) "yay"
string(1) "k"
string(1) "x"
string(3) "yay"
string(1) "k"
string(1) "y"
string(3) "yay"
string(1) "k"
string(1) "z"
string(3) "yay"
string(1) "k"
string(2) "zz"

Warning: Missing argument 2 for {closure}() in %s

Warning: Missing argument 4 for {closure}() in %s
string(3) "end"
