<?php

require_once 'Common.inc';

/**
 *Basic bucket tests
 */
class Cluster extends CouchbaseTestCommon
{
    function getClusterInfo($handle) {
        $str = $handle->getInfo();
        $this->assertNotNull($str, "Expected the server to return something");
        $obj = json_decode($str);
        $this->assertNotNull($obj, "Failed to decode the json output");
        return $obj;
    }

    function createHandle() {
        try {
            $cb = new CouchbaseClusterManager(COUCHBASE_CONFIG_HOST,
                COUCHBASE_CONFIG_USER,
                COUCHBASE_CONFIG_PASSWD);
            return $cb;
        } catch (CouchbaseException $e) {
            $this->assertEquals(
                                'CouchbaseClusterManager require username/password',
                                $e->getMessage());
            $this->markTestSkipped("Test skipped due to missing credentials");
        }
    }

    function testGetInfo() {
        $cb = $this->createHandle();
        if ($cb == null) {
            return;
        }
        $this->getClusterInfo($cb);
    }

    function doDeleteBucket($cb, $name) {
        try {
            $data = $cb->getBucketInfo($name);
        } catch (CouchbaseServerException $e) {
            if (preg_match("/Requested resource not found/", $e->getMessage())) {
                return;
            }
        }

        try {
            $cb->deleteBucket("php-ext-test-bucket");
        } catch (CouchbaseServerException $e) {
            $this->assertRegexp('/Some nodes are still deleting bucket/i',
                $e->getMessage());
        }

        do {
            try {
                $data = $cb->getBucketInfo($name);
            } catch (CouchbaseServerException $e) {
                if (preg_match("/Requested resource not found/",
                    $e->getMessage())) {
                        return;
                    }
            }
            sleep(1);
        } while (true);
    }


    function testBucketCreate() {
        $cb = $this->createHandle();
        if ($cb == null) {
            return;
        }

        $this->doDeleteBucket($cb, "php-ext-test-bucket");

        $info = $this->getClusterInfo($cb);
        $ram = $info->{"storageTotals"}->{"ram"};
        $used = $ram->{"quotaUsed"} / 1048576;
        $total = $ram->{"quotaTotal"} / 1048576;
        $space = $total - $used;

        if ($space > 100) {
            $v = $cb->createBucket("php-ext-test-bucket",
                array(
                      "type" => "couchbase",
                      "quota" => 100,
                      "replicas" => 1,
                      "enable_flush" => 1,
                      "parallel_compaction" => true,
                      "auth" => "sasl",
                      "password" => "foobar"
                    )
                );
            $this->assertTrue($v, "Expected to be able to create the bucket");
        } else {
            $this->markTestSkipped("Test skipped due to insufficient free space");
        }

        // Unfortunately the create is async, so we need to let it get
        // time to create itself.. It should be online when the cluster
        // populates the vbucket map
        do {
            sleep(1);
            $str = $cb->getBucketInfo("php-ext-test-bucket");
            $info = json_decode($str);
            $vbm = $info->{"vBucketServerMap"}->{"vBucketMap"};
        } while (count($vbm) < 1024);

	// Test flush as part of the create test ;)
        try {
            $cb->flushBucket("php-ext-test-bucket");
        } catch (CouchbaseException $e) {
            die($e->getMessage());
        }

        // Clean up after ourself
        $this->doDeleteBucket($cb, "php-ext-test-bucket");
    }

    function testInvalidParams() {
        $cb = $this->createHandle();
        if (!$cb) {
            return;
        }

        $this->assertExceptionRegexp(
            create_function('$cb', '$cb->createBucket(NULL, array());'),
            array($cb),
            '/./'
        );

        $this->assertExceptionRegexp(
            create_function('$cb',
                            '$cb->createBucket("a_bucket", array("foo"=>"bar"));'),
            array($cb),
            '/./'
        );

        $this->assertExceptionRegexp(
            create_function(
                '$cb',
                '$cb->createBucket('.
                    '"memcached_bucket",'.
                    'array("type" => "memcached", "replicas" => 4));'
            ),
            array($cb),
            '/./'
        );
    }
}

?>
