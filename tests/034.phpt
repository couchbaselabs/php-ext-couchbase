--TEST--
PCBC-91 Verify that custom views work correctly
--SKIPIF--
<?php include "skipif.inc" ?>
--FILE--
<?php
include "couchbase.inc";
// Setup a custom view first
$doc = array(
	'language' => 'javascript',
	'views' => array(
		'dev_testview' => array(
			'map' => 'function(doc) {emit(doc._id, 1)}',
			'reduce' => '_sum'
		)
	)
);
$context = stream_context_create(array('http' => array(
	'method' => 'PUT',
	'header' => 'Content-type: application/json',
	'content' => json_encode($doc)
)));

$uri = 'http://localhost:8092/default/_design/dev_testview';
$result = json_decode(file_get_contents($uri, false, $context), true);

if($result == $doc) {
	echo "Stored.\n";
}

$handle = new Couchbase(COUCHBASE_CONFIG_HOST, COUCHBASE_CONFIG_USER, COUCHBASE_CONFIG_PASSWD, COUCHBASE_CONFIG_BUCKET);

// drop all docs and store new ones
$result = $handle->view("_all_docs", "");
foreach ($result["rows"] as $key => $value) {
    $handle->delete($value["key"]);
}

sleep(1);

$handle->set('doc1', 1);
$handle->set('doc2', 2);

sleep(1);

// read data from the view with reduce
$result = $handle->view("dev_testview", "dev_testview");
var_dump($result);

// read data from the view without reduce
$result = $handle->view("dev_testview", "dev_testview?reduce=false");
var_dump($result);
?>
--EXPECTF--
Stored.
array(1) {
  ["rows"]=>
  array(1) {
    [0]=>
    array(2) {
      ["key"]=>
      NULL
      ["value"]=>
      int(2)
    }
  }
}
array(2) {
  ["total_rows"]=>
  int(2)
  ["rows"]=>
  array(2) {
    [0]=>
    array(3) {
      ["id"]=>
      string(4) "doc1"
      ["key"]=>
      NULL
      ["value"]=>
      int(1)
    }
    [1]=>
    array(3) {
      ["id"]=>
      string(4) "doc2"
      ["key"]=>
      NULL
      ["value"]=>
      int(1)
    }
  }
} 