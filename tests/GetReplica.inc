<?php

require_once 'Common.inc';

class GetReplica extends CouchbaseTestCommon {
    function testGetReplica() {
	$oo = $this->oo;
	$num = $oo->getNumReplicas();

	if ($num == 0) {
	   // @todo figure out how to print this out as skipped...
	   return ;
	}

	$key = $this->mk_key();
	try {
	    # Get at least 1 replica 1 replicas
	    $cas = $oo->add($key, "foobar", 0, 0, 1);
	} catch (CouchbaseException $e) {
	    $this->assertTrue(false, "Problem creating the replica");
	}

	$master_cas = "";
	$master = $oo->get($key, NULL, $master_cas);

	$replica_cas = "";
	$replica = $oo->getReplica($key, NULL, $replica_cas);

	$this->assertEquals($master_cas, $replica_cas, "CAS differ");
	$this->assertEquals($master, $replica, "value differ");
    }

    function testGetReplicaMulti() {
	$oo = $this->oo;
	$num = $oo->getNumReplicas();

	if ($num == 0) {
	   // @todo figure out how to print this out as skipped...
	   return ;
	}

	$ids = array($this->mk_key(), $this->mk_key(), $this->mk_key());
	foreach ($ids as $id) {
	    try {
	       # Get at least 1 replica 1 replicas
	       $cas = $oo->add($id, "foobar", 0, 0, 1);
	    } catch (CouchbaseException $e) {
		$this->assertTrue(false, "Problem creating the replica");
	    }
	}

	$master_cas = array();
	$master = $oo->getMulti($ids, $master_cas);

	$replica_cas = array();
	$replica = $oo->getReplicaMulti($ids, $replica_cas);

	$cas_diff = array_diff($master_cas, $replica_cas);
	$val_diff = array_diff($master, $replica);

	$this->assertTrue(empty($cas_diff));
	$this->assertTrue(empty($val_diff));
    }
}
