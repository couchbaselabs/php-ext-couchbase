<?php
require_once 'Common.inc';

/* This class is intended to replace 006.phpt from the older testing scheme. */

class Flush extends CouchbaseTestCommon {
	/**
	 * Common core test setup for get operations (oo)
	 */
	function _test_setup_common_oo() {
		$oo = $this->oo;

		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');
		
		$cas = $oo->set($key, $val);

		return array($oo, $key, $val, $cas);
	}

	/**
	 * Common core test setup for get operations (non-oo)
	 */
	function _test_setup_common() {
		$h = $this->handle;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');

		$cas = couchbase_set($h, $key, $val);

		return array($h, $key, $val, $cas);
	}

	/**
	 * @test flush
	 *
	 * @pre
	 * setup run
	 *
	 * @post
	 * key is not set
	 */
	function testFlush() {
		$this->markTestSkipped("Flush command is currently inaccessible");

		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();

		$oo->flush();

		$this->assertNull($oo->get($key));
	}

	/**
	 * @test flush (oo)
	 *
	 * @pre 
	 * setup run
	 *
	 * @post
	 * key is not set
	 */
	function testFlushOO() {
		$this->markTestSkipped("Flush command is currently inaccessible");

		list($h, $key, $val, $cas) = $this->_test_setup_common();

		couchbase_flush($h);

		$this->assertNull(couchbase_get($h, $key));
	}
}
?>
