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
     * - pure functional api: E_WARNING raised, false returned
     * - OO: E_WARNING raised, object handle can have getResultCode/Message() called
     *
     * @variants
     * pure functional, OO
     *
     * @test_plans{1.3}
     */
    function testConnectBad() {
        // first, make sure the warning gets raised:
        // (the default error handler from CBTestFramework will turn it into an exception)
        try {
            $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                        'bad_username',
                                        'bad_password',
                                        COUCHBASE_CONFIG_BUCKET);
        } catch (Exception $e) {
            $this->assertRegexp('/failed/i', $e->getMessage());
        }

        try {
            $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                        COUCHBASE_CONFIG_USER,
                                        COUCHBASE_CONFIG_PASSWD,
                                        "non-existent-bucket....blah...");
        } catch (Exception $e) {
            $this->assertRegexp('/failed/i', $e->getMessage());
        }

        // now, make sure the value would be false:
        $this->setErrorsRaiseException(false);

        $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                    'bad_username',
                                    'bad_password',
                                    COUCHBASE_CONFIG_BUCKET);

        $this->assertFalse($handle,
                           "Invalid auth credentials make for a false ".
                           "connection handle");

        $handle = couchbase_connect(COUCHBASE_CONFIG_HOST,
                                    COUCHBASE_CONFIG_USER,
                                    COUCHBASE_CONFIG_PASSWD,
                                    "non-existent-bucket....blah...");
        $this->assertFalse($handle,
                           "Invalid bucket makes for a false connection handle");

        $this->setErrorsRaiseException(true);
    }

    /** @test_plans{1.1} */
    function testConnectOO() {
        $handle = new Couchbase(COUCHBASE_CONFIG_HOST,
                                COUCHBASE_CONFIG_USER,
                                COUCHBASE_CONFIG_PASSWD,
                                COUCHBASE_CONFIG_BUCKET);
        $this->assertInstanceOf('Couchbase', $handle, 'Item is a Couchbase (OO)');
    }

    /**
     * @test testConnectBadOO
     * @see testConnectBad
     */
    function testConnectBadOO() {
        // first, check that warning is thrown (see comments in testConnectBad)
        try {
            $oo = new Couchbase('http://999.999.999.999:9999',
                                'bad-user',
                                'bad-password',
                                'does-not-exist');

        } catch (Exception $e) {
            $this->assertRegExp('/failed/i', $e->getMessage());
        }

        // now, make sure the object would support the given operations:
        $this->setErrorsRaiseException(false);

        $oo = new Couchbase('http://999.999.999.999:9999',
                            'bad-user',
                            'bad-password',
                            'does-not-exist');

        $this->assertGreaterThan(0, $oo->getResultCode());
        $this->assertNotNull($oo->getResultMessage());
        $this->assertNotEquals("", $oo->getResultMessage());

        $this->setErrorsRaiseException(true);
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
        $this->assertExceptionRegexp(function ($u) { return couchbase_connect($u);},
                                     array($url),
                                     "/failed/i");

        $this->assertExceptionRegexp(function ($u) { return new Couchbase($u); },
                                     array($url),
                                     "/failed/i");

        $this->setErrorsRaiseException(false); // disable exception-raising error handler

        $h = couchbase_connect($url);
        $this->assertFalse($h);

        $o = new Couchbase($url);
        $this->assertEquals(23, $o->getResultCode()); // 23 = 0x17 = LCB_CONNECT_ERROR
        $this->assertContains('Connection', $o->getResultMessage());

        # Try with a malformed uri
        $this->setErrorsRaiseException(true);
        $url = "http://";
        $this->assertExceptionRegexp(function ($u) { return couchbase_connect($u); },
                                     array($url),
                                     "/malformed/i");

        $this->assertExceptionRegexp(function($u) { return new Couchbase($u); },
                                     array($url),
                                     "/malformed/i");

        $this->setErrorsRaiseException(false); // then disable again
        
        # try with an array. (not sure if we need to check for connection
        # warnings yet again here,
        # as it'd all be going through the same C code path re: connection...)
        if ($this->atLeastVersion(array(1,0,5))) {
            $hosts = array("http://127.0.0.1:1");

            $h = couchbase_connect($hosts);
            $this->assertFalse($h);

            $o = new Couchbase($hosts);
            $this->assertEquals(23, $o->getResultCode()); // see note above re: 23
            $this->assertContains('Connection', $o->getResultMessage());
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
            $o = couchbase_connect(COUCHBASE_CONFIG_HOST,
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
