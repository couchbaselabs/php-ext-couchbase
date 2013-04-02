<?php

$cb = new CouchbaseClusterManager("localhost", "Administrator", "password");

$cb->createBucket("mybucket", array("type" => "couchbase",
 "quota" => 256,
 "replicas" => 1,
 "enable flush" => 1,
 "parallel compaction" => true,
 "auth" => "none",
 "port" => 11212));

$cb->modifyBucket("mybucket", array("type" => "couchbase",
 "quota" => 512,
 "replicas" => 1,
 "enable flush" => 1,
 "parallel compaction" => true,
 "auth" => "none",
 "port" => 11212));

$cb->deleteBucket("mybucket");
?>
