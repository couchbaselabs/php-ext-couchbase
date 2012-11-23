<?php

require_once 'Common.inc';

class Sync extends CouchbaseTestCommon {

    function testIllegalNumberOfReplicas() {
        $oo = $this->oo;
        $key = $this->mk_key();
        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->add($key, "foobar", 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->add($key, "foobar", 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->add($key, "foobar", 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }

        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->set($key, "foobar", 0, 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->set($key, "foobar", 0, 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->set($key, "foobar", 0, 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }

        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->replace($key, "foobar", 0, 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->replace($key, "foobar", 0, 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->replace($key, "foobar", 0, 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }

        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->append($key, "foobar", 0, 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->append($key, "foobar", 0, 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->append($key, "foobar", 0, 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }

        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->prepend($key, "foobar", 0, 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->prepend($key, "foobar", 0, 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->prepend($key, "foobar", 0, 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }

        try {
            # persist on 100 nodes, 0 replicas
            $cas = $oo->delete($key, 0, 100, 0);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 0 nodes, 100 replicas
            $cas = $oo->delete($key, 0, 0, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
        try {
            # persist on 100 nodes, 100 replicas
            $cas = $oo->delete($key, 0, 100, 100);
            $this->assertTrue(false, "We should get the exception");
        } catch (CouchbaseNotEnoughNodesException $e) {

        }
    }

    function testAddOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        try {
            # persist on 1 node
            $cas = $oo->add($key, "foobar", 0, 1);
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testAppendOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");

        try {
            # persist on 1 node
            $cas2 = $oo->append($key, "foobar", 0, $cas, 1);
            $this->assertNotEquals($cas, $cas2, "append should change cas");
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testCasOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");
        try {
            # persist on 1 node
            $oo->cas($cas, $key, "foobar", 0, 1);
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testPrependOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");
        try {
            # persist on 1 node
            $cas2 = $oo->prepend($key, "foobar", 0, $cas, 1);
            $this->assertNotEquals($cas, $cas2, "prepend should change cas");
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testRemoveOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");
        try {
            # persist on 1 node
            $cas2 = $oo->delete($key, $cas, 1);
            $this->assertNotEquals($cas, $cas2, "remove should change cas");
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testReplaceOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");
        try {
            # persist on 1 node
            $cas2 = $oo->replace($key, "foobar", 0, $cas, 1);
            $this->assertNotEquals($cas, $cas2, "replace should change cas");
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

    function testSetOnePersist() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->add($key, "foobar");
        try {
            # persist on 1 node
            $cas2 = $oo->set($key, "foobar", 0, $cas, 1);
            $this->assertNotEquals($cas, $cas2, "set should change cas");
        } catch (CouchbaseNotEnoughNodesException $e) {
            $this->assertTrue(false, "We should at least have a single node");
        } catch (CouchbaseTimeoutException $e) {
            # this could happen
        } catch (CouchbaseException $e) {
            $this->assertTrue(false, $e->getMessage());
        }
    }

}
