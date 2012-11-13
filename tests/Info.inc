<?php
require_once 'Common.inc';

class Info extends CouchbaseTestCommon {

    function skipALL() {
        parent::skipALL();
        if (!$this->atLeastVersion(array(1,1,0))) {
            $this->markTestSkipped("numReplicas/servers implemented in 1.1");
        }
    }

    /**
     * @test Getting the number of replicas
     * @pre call the num_replicas function
     * @post return value is a numeric type
     */
    function testNumReplicas() {
        $h = $this->getPersistHandle();
        $rv = couchbase_get_num_replicas($h);
        $this->assertInternalType('integer', $rv);
    }

    function testServerList() {
        $h = $this->getPersistHandle();
        $servers = couchbase_get_servers($h);

        $this->assertInternalType('array', $servers);
        $this->assertGreaterThan(0, count($servers));
        foreach ($servers as $srv) {
            $this->assertNotEmpty($srv);
            $this->assertInternalType('string', $srv);
        }
    }


    function testNumReplicasOO() {
        $oo = $this->getPersistOO();
        $rv = $oo->getNumReplicas();
        $this->assertInternalType('integer', $rv);
    }

    function testServerListOO() {
        $oo = $this->getPersistOO();
        $rv = $oo->getServers();
        $this->assertInternalType('array', $rv);
        foreach ($rv as $srv) {
            $this->assertNotEmpty($srv);
            $this->assertInternalType('string', $srv);
        }
    }

    /**
     * Test that we may get and set the timeout values from
     * libcouchbase. We don't try to test the actual timeout
     * functionality in libcouchbase, but assume that if we're
     * able to modify it its good enough for us (the actual
     * timeout code in libcouchbase should be tested there).
     *
     * Test both the object oriented and the functional interface
     */
    function testGetSetTimeout() {
        $h = $this->getPersistHandle();
        $tmo = couchbase_get_timeout($h);
	$next = $tmo + 1;
	$this->assertTrue(couchbase_set_timeout($h, $next));
	$this->assertEquals($next, couchbase_get_timeout($h));

	// Restore original value
	$this->assertTrue(couchbase_set_timeout($h, $tmo));
    }

    function testGetSetTimeoutOO() {
        $oo = $this->getPersistOO();
        $tmo = $oo->getTimeout();
	$next = $tmo + 1;
	$this->assertTrue($oo->setTimeout($next));
	$this->assertEquals($next, $oo->getTimeout());

	// Restore original value
	$this->assertTrue($oo->setTimeout($tmo));
    }
}
