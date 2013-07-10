<?php

require_once 'Common.inc';

class GetReplica extends CouchbaseTestCommon {

    /**
     * We can't run the replica tests unless we've got a real cluster
     * with enough replicas configured (and enough servers to store them
     * on.
     *
     * @param type $cb a Couchbase object
     * @return boolean true if we should skip the tests
     */
    function doSkip($cb) {
        $num = $cb->getNumReplicas();
        $numServers = count($cb->getServers());
        if ($num == 0 || $numServers <= $num) {
            // @todo figure out how to print this out as skipped...
            return true;
        }
        return false;
    }

    function testIllegalStrategies() {
        $cb = $this->oo;
        try {
            $cb->getReplica("", 5);
            $this->assertTrue(false, "5 is not a legal replica strategy");
        } catch (Exception $e) {

        }

        try {
            $cb->getReplica("", "first");
            $this->assertTrue(false, "\first\" is not a legal replica strategy");
        } catch (Exception $e) {

        }

        try {
            $cb->getReplica("", array(""));
            $this->assertTrue(false, "empty array is not a legal replica strategy");
        } catch (Exception $e) {

        }

        try {
            $cb->getReplica("", array("blah"));
            $this->assertTrue(false, "array with blah is not a legal replica strategy");
        } catch (Exception $e) {

        }

        try {
            $cb->getReplica("", array("strategy" => "yalla"));
            $this->assertTrue(false, "yalla strategy is not a legal replica strategy");
        } catch (Exception $e) {

        }

        try {
            $cb->getReplica("", array("strategy"));
            $this->assertTrue(false, "empty strategy is not a legal replica strategy");
        } catch (Exception $e) {

        }
    }

    function testGetReplicaDefault() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key = $this->mk_key();
        try {
            $cb->add($key, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        try {
            $obj = $cb->getReplica($key);
            $data = $obj[$key];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetReplicaStrategyFirst() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key = $this->mk_key();
        try {
            $cb->add($key, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        try {
            $obj = $cb->getReplica($key, COUCHBASE_REPLICA_FIRST);
            $data = $obj[$key];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($key, "COUCHBASE_REPLICA_FIRST");
            $data = $obj[$key];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($key, array("strategy" => "first"));
            $data = $obj[$key];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetReplicaStrategyAll() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key = $this->mk_key();
        try {
            $cb->add($key, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        try {
            $obj = $cb->getReplica($key, COUCHBASE_REPLICA_ALL);
            $object = $obj[$key];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($key, "COUCHBASE_REPLICA_ALL");
            $object = $obj[$key];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($key, array("strategy" => "all"));
            $object = $obj[$key];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetReplicaStrategySelect() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key = $this->mk_key();
        try {
            $cb->add($key, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        try {
            $obj = $cb->getReplica($key, array("strategy" => "select", "index" => 0));
            $data = $obj[$key];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetMultipleReplicasDefault() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key1 = $this->mk_key();
        $key2 = $this->mk_key();
        try {
            $cb->add($key1, "foobar", 0, 0, 1);
            $cb->add($key2, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        try {
            $obj = $cb->getReplica(array($key1, $key2));
            $data = $obj[$key1];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $data = $obj[$key2];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetMultipleReplicasStrategyFirst() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key1 = $this->mk_key();
        $key2 = $this->mk_key();
        try {
            $cb->add($key1, "foobar", 0, 0, 1);
            $cb->add($key2, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }
        $keys = array($key1, $key2);

        try {
            $obj = $cb->getReplica($keys, COUCHBASE_REPLICA_FIRST);
            $data = $obj[$key1];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $data = $obj[$key2];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($keys, "COUCHBASE_REPLICA_FIRST");
            $data = $obj[$key1];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $data = $obj[$key2];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($keys, array("strategy" => "first"));
            $data = $obj[$key1];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $data = $obj[$key2];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetMultipleReplicasStrategyAll() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key1 = $this->mk_key();
        $key2 = $this->mk_key();
        try {
            $cb->add($key1, "foobar", 0, 0, 1);
            $cb->add($key2, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        $keys = array($key1, $key2);

        try {
            $obj = $cb->getReplica($keys, COUCHBASE_REPLICA_ALL);
            $object = $obj[$key1];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $object = $obj[$key2];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($keys, "COUCHBASE_REPLICA_ALL");
            $object = $obj[$key1];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $object = $obj[$key2];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }

        try {
            $obj = $cb->getReplica($keys, array("strategy" => "all"));
            $object = $obj[$key1];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $object = $obj[$key2];
            $data = $object[0];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

    function testGetMultipleReplicasStrategySelect() {
        $cb = $this->oo;
        if ($this->doSkip($cb)) {
            // @todo figure out how to mark it as skipped
            return;
        }

        $key1 = $this->mk_key();
        $key2 = $this->mk_key();
        try {
            $cb->add($key1, "foobar", 0, 0, 1);
            $cb->add($key2, "foobar", 0, 0, 1);
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, "Problem creating the replica");
        }

        $keys = array($key1, $key2);

        try {
            $obj = $cb->getReplica($keys, array("strategy" => "select", "index" => 0));
            $data = $obj[$key1];
            $this->assertEquals("foobar", $data["value"], "value differ");
            $data = $obj[$key2];
            $this->assertEquals("foobar", $data["value"], "value differ");
        } catch (Exception $e) {
            $this->assertTrue(false, "Problem creating the replica " . $e->getMessage());
        }
    }

}
