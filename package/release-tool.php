#!/usr/bin/env php
<?php

$VERSIONS = array(
	"1.1.3" => "php-ext-couchbase",
	"1.1.2" => "php-ext-couchbase",
	"1.1.1" => "php-ext-couchbase",
	"1.1.0" => "php-ext-couchbase",
	"1.0.0" => "php-ext-couchbase-1.0",
	"1.0.2" => "php-ext-couchbase-1.0",
	"1.0.3" => "php-ext-couchbase-1.0",
	"1.0.4" => "php-ext-couchbase-1.0",
	"1.0.5" => "php-ext-couchbase-1.0");
$OSes = array("centos55", "centos62", "ubuntu1004", "ubuntu1110");
$BITs = array("32" => "i686","64" => "x86_64");
# note, macos is php-ext-couchbase-$VERSION-macosx-x86_64.tar.gz

foreach($VERSIONS AS $VERSION => $BUILDER) {
  $srccmd = "git archive --format=tar.gz --prefix=php-ext-couchbase-$VERSION/ $VERSION > php-ext-couchbase-$VERSION.tar.gz";
  echo $srccmd . PHP_EOL;
  echo `$srccmd`;
}

foreach($VERSIONS AS $VERSION => $BUILDER) {
	foreach($OSes AS $OS) {
		foreach($BITs AS $BIT => $LABEL) {
			$filename = "php-ext-couchbase-$VERSION-$OS-$LABEL.tar.gz";
			$url = "http://sdkbuilds.couchbase.com/job/sdk-repo/label=sdk_{$OS}_{$BIT}/ws/php/package/php-ext-couchbase.tar.gz";
			$cmd = "curl -so '$filename' '$url'";
			echo $cmd . PHP_EOL;
			echo `$cmd`;
		}
	}
}

echo "\nUpload with s3cmd put --acl-public php-ext-couchbase-* s3://packages.couchbase.com/clients/php/ \n";

// http://sdkbuilds.couchbase.com/job/sdk-repo/label=sdk_centos55_64/ws/php/package/php-ext-couchbase.tar.gz
