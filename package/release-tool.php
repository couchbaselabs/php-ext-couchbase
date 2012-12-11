#!/usr/bin/env php
<?php

$VERSIONS = array(
	"1.1.0" => "php-ext-couchbase",
	"1.0.5" => "php-ext-couchbase-1.0");
$OSes = array("centos55", "centos62", "ubuntu1004", "ubuntu1110");
$BITs = array("32" => "i686","64" => "x86_64");

foreach($VERSIONS AS $VERSION => $BUILDER) {
	foreach($OSes AS $OS) {
		foreach($BITs AS $BIT => $LABEL) {
			$filename = "php-ext-couchbase-$VERSION-$OS-$LABEL.tar.gz";
			$url = "http://sdkbuilds.couchbase.com/job/$BUILDER/label=sdk_{$OS}_{$BIT}/ws/package/php-ext-couchbase.tar.gz";
			$cmd = "curl -so '$filename' '$url'";
			echo $cmd . "\n";
			echo `$cmd`;
		}
	}
}

echo "\nUpload with s3cmd put --acl-public php-ext-couchbase-* s3://packages.couchbase.com/clients/php/ \n";

// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_centos55_32/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_centos55_64/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_centos62_32/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_centos62_64/ws/package/php-ext-couchbase.tar.gz
// 
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_ubuntu1004_32/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_ubuntu1004_64/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_ubuntu1110_32/ws/package/php-ext-couchbase.tar.gz
// http://sdkbuilds.couchbase.com/job/php-ext-couchbase/label=sdk_ubuntu1110_64/ws/package/php-ext-couchbase.tar.gz
// 
// 
// http://packages.couchbase.com.s3.amazonaws.com/clients/php/php-ext-couchbase-1.1.0-dp1-centos-i686.tar.gz
// http://packages.couchbase.com.s3.amazonaws.com/clients/php/php-ext-couchbase-1.1.0-dp1-centos-x86_64.tar.gz
// http://packages.couchbase.com.s3.amazonaws.com/clients/php/php-ext-couchbase-1.1.0-dp1-ubuntu-i686.tar.gz
// http://packages.couchbase.com.s3.amazonaws.com/clients/php/php-ext-couchbase-1.1.0-dp1-ubuntu-x86_64.tar.gz