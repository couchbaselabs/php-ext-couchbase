<?php
require_once 'Common.inc';

/* This class is intended to replace 005.phpt from the older testing scheme. */

class Get extends CouchbaseTestCommon {
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

    function _v11_feature() {
        if (!$this->atLeastVersion(array(1,1))) {
            $this->markTestSkipped("Need v1.1 or higher");
        }
    }

	/**
	 * @test
	 * Get (plain)
	 *
	 * @pre
	 * Runs @ref _test_common.
	 *
	 * @post
	 * Common test works.
	 *
	 * @remark
	 * Variants: OO, plain
	 */
	function testPlain() {
		list($h, $key, $val, $cas) = $this->_test_setup_common();

		$valGet = couchbase_get($h, $key, $casGet);

		$this->assertEquals($val, $valGet);
		$this->assertEquals($cas, $casGet);

		couchbase_delete($h, $key);
	}

	/**
	 * @test
	 * Get (OO)
	 *
	 * @pre
	 * runs @ref _test_common_oo.
	 *
	 * @post
	 * Common test (OO) works.
	 *
	 * @remark
	 * Variants: OO, plain
	 */
	function testPlainOO() {
		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();

		$valGet = $oo->get($key, $casGet);

		$this->assertEquals($val, $valGet);
		$this->assertEquals($cas, $casGet);

		$oo->delete($key);
	}

	/**
	 * @test get non-existent key (oo)
	 *
	 * @pre
	 * Setup run
	 *
	 * @post
	 * Get on non-existent key returns null.
	 *
	 * @variants
	 * OO, plain
	 */
	function testNonExistentKeyOO() {
		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();
		$oo->delete($key);

		$this->assertNull($oo->get($key));
	}

	/**
	 * @test get non-existent key (plain)
	 *
	 * @pre
	 * setup run
	 *
	 * @post
	 * get on non-existent key returns null
	 *
	 * @variants
	 * OO, plain
	 */
	function testNonExistentKey() {
		list($h, $key, $val, $cas) = $this->_test_setup_common();
		couchbase_delete($h, $key);

		$this->assertNull(couchbase_get($h, $key));
	}

	/**
	 * @test get-with-touch
	 * @group slow
	 *
	 * @pre
	 * Setup run
	 *
	 * @post
	 * cas unchanged with touch, key is null after expiry
	 */
	function testGetWithTouch() {
		$this->skipGetWithTouch();

		list($h, $key, $val, $cas) = $this->_test_setup_common();

		$valGet = couchbase_get_and_touch($h, $key, 1, $expCAS);

		$this->assertEquals($val, $valGet);
		$this->assertEquals($cas, $expCAS);

		sleep(2);

		$this->assertNull(couchbase_get($h, $key));
	}

	function skipGetWithTouch() { $this->_v11_feature(); }

	/**
	 * @test get-with-touch OO
	 * @group slow
	 *
	 * @pre
	 * Setup run
	 *
	 * @post
	 * cas unchanged with touch, key is null after expiry
	 */
	function testGetWithTouchOO() {
		$this->skipGetWithTouchOO();

		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();

		$valGet = $oo->getAndTouch($key, 1, $expCAS);

		$this->assertEquals($val, $valGet);
		$this->assertEquals($cas, $expCAS);

		sleep(2);

		$this->assertNull($oo->get($key));
	}

	function skipGetWithTouchOO() { $this->_v11_feature(); }

	/**
	 * @test get-with-lock (oo)
	 * @group slow
	 *
	 * @pre
	 * Setup run
	 *
	 * @post
	 * cas changed with lock, key deletable after lock expires
	 */
	function testGetWithLockOO() {
		$this->skipGetWithLockOO();

		list($oo, $key, $val, $cas) = $this->_test_setup_common_oo();

		$valGet = $oo->getAndLock($key, $lockCAS, 1);

		$this->assertEquals($val, $valGet);
		$this->assertNotEquals($cas, $lockCAS);

		sleep(2);

		$cas = $oo->delete($key);
		$this->assertTrue(is_string($cas));
	}

	function skipGetWithLockOO() { $this->_v11_feature(); }

	/**
	 * @test get-with-lock
	 * @group slow
	 *
	 * @pre
	 * Setup run
	 *
	 * @post
	 * cas changed with lock, key deletable after lock expires
	 */
	function testGetWithLock() {
		$this->skipGetWithLock();

		list($h, $key, $val, $cas) = $this->_test_setup_common();

		$valGet = couchbase_get_and_lock($h, $key, $lockCAS, 1);

		$this->assertEquals($val, $valGet);
		$this->assertNotEquals($cas, $lockCAS);

		sleep(2);
                $cas = couchbase_delete($h, $key);
		$this->assertTrue(is_string($cas));
	}
	function skipGetWithLock() { $this->_v11_feature(); }
}
?>
