<?php

require_once 'Common.inc';
/**
 *Basic connection tests, replaces 002, 028 036
 */

class Connection extends CouchbaseTestCommon
{
    /**
     * @test
     * Basic Connection
     *
     * @pre
     * Connect using couchbase_connect to the specified bucket,
     * host, and auth creds
     *
     * @post
     * newly initialized couchbase objects
     *
     * @remark
     * Variants: OO (testConnectBasic), URI (testConnectUri)
     *
     * @test_plans{1.1}
     */
    function testConnectBasic() {
        $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                    COUCHBASE_CONFIG_USER,
                                    COUCHBASE_CONFIG_PASSWD,
                                    COUCHBASE_CONFIG_BUCKET);

        $this->assertInternalType('resource', $handle, 'Couchbase Resource');
    }

    /**
     * @test
     * Connection (Failed Auth)
     *
     * We would like to be able to expect an exception, but this does not seem
     * possible currently.. (we don't have classes for exceptions)
     *
     * @pre
     * Connect with a bad username/password
     *
     * @post
     * Appropriate error message
     *
     * @test_plans{1.3}
     */
    function testConnectBad() {
        $handle = NULL;
        try {
            $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                        'bad_username',
                                        'bad_password',
                                        COUCHBASE_CONFIG_BUCKET);
        } catch (Exception $exc) {
            $this->assertRegExp("/auth/i", $exc->getMessage());
        }

        $this->assertNull($handle, "Bad connection parameters makes NULL handle");

        try {
            $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                        COUCHBASE_CONFIG_USER,
                                        COUCHBASE_CONFIG_PASSWD,
                                        "non-existent-bucket....blah...");
        } catch (Exception $exc) {

        }
        $this->assertNull($handle, "Invalid bucket OK");
    }

    /** @test_plans{1.1} */
    function testConnectOO() {
        $handle = new Couchbase(COUCHBASE_CONFIG_HOST,
                                COUCHBASE_CONFIG_USER,
                                COUCHBASE_CONFIG_PASSWD,
                                COUCHBASE_CONFIG_BUCKET);
        $this->assertInstanceOf('Couchbase', $handle, 'Item is a Couchbase (OO)');
    }


    function testConnectUri() {
        $url = sprintf("htTp://%s:%s@%s/%s",
                        COUCHBASE_CONFIG_USER,
                        COUCHBASE_CONFIG_PASSWD,
                        COUCHBASE_CONFIG_HOST,
                        COUCHBASE_CONFIG_BUCKET);
        $handle = couchbase_connect($url);
        $this->assertInternalType('resource', $handle);
        $handle = new Couchbase($url);
        $this->assertInstanceOf('Couchbase', $handle);
    }

    /**
     * @test
     * Connection (Bad URI)
     *
     * @pre
     * Connect using bad URIs for the clients to connect to
     *
     * @post
     * error messages for each
     *
     * @remarks
     * Variants: bad host, malformed URI, array
     * @bug PCBC-74
     *
     * @test_plans{1.2}
     */
    function testConnectBadUri() {
        # PCBC-74
        $url = "http://127.0.0.1:1";
        $exmsg = NULL;
        try {
            $o = new Couchbase($url);
        } catch (Exception $exc) {
            $exmsg = $exc->getMessage();
        }
        # No segfaults? we're good!
        $this->assertNotNull($exmsg);
        $this->assertContains('Failed to connect', $exmsg);

        # Try with a malformed uri
        $exmsg = NULL;
        try {
            $url = "http://";
            $o = new Couchbase($url);
        } catch (Exception $exc) {
            $exmsg = $exc->getMessage();
        }
        $this->assertNotNull($exmsg);
        $this->assertContains('malformed', $exmsg);

        # try with an array.
        if ($this->atLeastVersion(array(1,0,5))) {
            $exmsg = NULL;
            try {
                $o = new Couchbase(
                    array("http://127.0.0.1:1")
                );
            } catch (Exception $exc) {
                $exmsg = $exc->getMessage();
            }
            $this->assertNotNull($exmsg);
            $this->assertContains('Failed to connect', $exmsg);
        }
    }

    /**
     * @test
     * connection (Multi Node)
     *
     * @pre
     * create an array of three hostnames, first two are bogus, second is
     * value. Pass the array to the ctor
     *
     * @post
     * connection succeeds
     *
     * @remark
     * Variants: semicolon-delimited nodes
     *
     * @test_plans{1.5}
     */
    function testConnectNodeArray() {
        if (!$this->atLeastVersion(array(1,0,5))) {
            return;
        }
        $hosts = array('non-existent-host',
                       'another-bogus-host',
                       COUCHBASE_CONFIG_HOST);

        $cb = new Couchbase($hosts,
                            COUCHBASE_CONFIG_USER,
                            COUCHBASE_CONFIG_PASSWD,
                            COUCHBASE_CONFIG_BUCKET);
        $this->assertInstanceOf('Couchbase', $cb);
    }

    /**
     * @test_plans{1.5}
     */
    function testConnectDelimitedNodes() {

        $hosts = array('non-existent-host',
                       'another-bogus-host',
                       COUCHBASE_CONFIG_HOST);

        $host_str = implode(';', $hosts);

        $cb = new Couchbase($host_str,
                            COUCHBASE_CONFIG_USER,
                            COUCHBASE_CONFIG_PASSWD,
                            COUCHBASE_CONFIG_BUCKET);

        $this->assertInstanceOf('Couchbase', $cb);
    }

    /**
     * @test
     * Test persistent connection feature
     *
     * @pre
     * Create a connection, specifying 'true' for persistence. Create a second
     * connection with the same paranmeters.
     *
     * @post
     * no crashes
     *
     * @bug PCBC-75
     *
     */
    function testPersistentConnection() {
        for ($ii = 0; $ii < 2; $ii++) {
            $o =  couchbase_connect(COUCHBASE_CONFIG_HOST,
                                COUCHBASE_CONFIG_USER,
                                COUCHBASE_CONFIG_PASSWD,
                                COUCHBASE_CONFIG_BUCKET,
                                true);
            $obj[$ii] = $o;
        }

        $this->assertTrue(true, "No problems so far..");

        $this->markTestIncomplete(
            "No way of verifying the objects are indentical");

        $this->assertEquals($obj[0], $obj[1],
                            "Persistent connection returns ".
                            "same resource object");
    }
}



?>
