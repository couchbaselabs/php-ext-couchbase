<?php
// This example file shows you how to create, get and delete a view

try {
   $cb = new Couchbase("localhost", "Administrator", "asdasd");

   $viewname = "myview";
   $designdoc = "mydesign";

   // Create the map function.
   $func = "function (doc, meta) { emit(meta.id, NULL); }";

   // Create document containing the map function
   $ddoc = json_encode('{"views":{"' . $viewname .
                       '":{"map":"' . $func . '"}}}');

   // Create the design document on the server
   $ret = $cb->setDesignDoc($designdoc, json_decode($ddoc));
   if ($ret) {
      print "View successfully created" . PHP_EOL;
   } else {
      print "Failed to create view: " . $cb->getResultMessage() . PHP_EOL;
   }

   // Try to retrieve the desgin document:
   $ddoc = $cb->getDesignDoc($designdoc);
   print "The design document looks like: " . PHP_EOL;
   var_dump($ddoc);

   // Delete the design document:
   $ret = $cb->deleteDesignDoc($designdoc);
   if ($ret) {
      print "View successfully deleted" . PHP_EOL;
   } else {
      print "Failed to delete view: " . $cb->getResultMessage() . PHP_EOL;
   }

} catch (CouchbaseException $exp) {
   print "Failed to create view: " . $exp->getMessage() . PHP_EOL;
   var_dump($exp);
}
?>