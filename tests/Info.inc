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
}
