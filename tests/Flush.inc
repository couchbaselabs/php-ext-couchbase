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
	 * @test flush (oo)
	 *
	 * @pre
	 * setup run
	 *
	 * @post
	 * key is not set
	 */
	function testFlushOO() {
		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();
		try {
			$oo->flush();
			$this->assertNull($oo->get($key));
		} catch (CouchbaseAuthenticationException $exp) {
			$this->markTestSkipped("Flush command is currently inaccessible (need auth)");
		} catch (CouchbaseServerException $exp) {
			if (preg_match("/Flush is disabled for the bucket/", $exp->getMessage())) {
				$this->markTestSkipped("Flush command is currently inaccessible (not enabled)");
			} else {
				die("Failed to flush bucket: " . $exp->getMessage() . "\n");
			}
		}
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
		list($h, $key, $val, $cas) = $this->_test_setup_common();

		couchbase_flush($h);

		$this->assertNull(couchbase_get($h, $key));
	}
}
?>
