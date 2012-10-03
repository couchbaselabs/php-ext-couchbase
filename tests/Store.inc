<?php
require_once 'Common.inc';

/* This class tests add and set operations, both singular and multiple values.
Ultimately these operations go through php_couchbase_store_impl. */

class Store extends CouchbaseTestCommon {
	/**
	 * @test
	 * Add (plain)
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * adding a single item works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testAddSingle() {
		$h = $this->handle;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');
	
		$cas = couchbase_add($h, $key, $val);
		$valGet = couchbase_get($h, $key, NULL, $casGet);

		$this->assertEquals($valGet, $val);
		$this->assertEquals($casGet, $cas);
	}
	
	/**
	 * @test
	 * Add (OO)
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * adding a single item works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testAddSingleOO() {
		$oo = $this->oo;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');

		$cas = $oo->add($key, $val);
		$valGet = $oo->get($key, NULL, $casGet);

		$this->assertEquals($valGet, $val);
		$this->assertEquals($casGet, $cas);
	}
	
	/**
	 * @test
	 * Add on Key that Already Exists
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * adding a key that already exists does not modify the prior value
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testAddWhenExists() {
		$h = $this->handle;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');

		couchbase_add($h, $key, $val);

		$val2 = uniqid('another-couchbase-value-');

		couchbase_add($h, $key, $val2);

		$v = couchbase_get($h, $key);

		$this->assertEquals($v, $val);
		$this->assertNotEquals($v, $val2);
	}
	
	/**
	 * @test
	 * Add on Key that Already Exists (OO)
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * adding a key that already exists does not modify the prior value
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testAddWhenExistsOO() {
		$oo = $this->oo;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');

		$oo->add($key, $val);

		$val2 = uniqid('another-couchbase-value-');
		
		$oo->add($key, $val2);

		$v = $oo->get( $key);

		$this->assertEquals($v, $val);
		$this->assertNotEquals($v, $val2);
	}

	/**
	 * @test
	 * Set 
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * setting a single item works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testSetSingle() {
		$h = $this->handle;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');
		
		$cas = couchbase_set($h, $key, $val);
		$valGet = couchbase_get($h, $key, NULL, $casGet);

		$this->assertEquals($valGet, $val);
		$this->assertEquals($casGet, $cas);
	}

	/**
	 * @test
	 * Set (OO)
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * setting a single item works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testSetSingleOO() {
		$oo = $this->oo;
		$key = $this->mk_key();
		$val = uniqid('couchbase-value-');

		$cas = $oo->set($key, $val);
		$valGet = $oo->get($key, NULL, $casGet);

		$this->assertEquals($valGet, $val);
		$this->assertEquals($casGet, $cas);
	}
	
	/**
	 * @test
	 * Set Multi
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * setting several items at once works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testSetMulti() {
		$h = $this->handle;
		$keys = array();
		for ($i = 0; $i < 3; $i++) {
			$keys[$this->mk_key()] = uniqid('couchbase-value-');
		}

		couchbase_set_multi($h, $keys);

		foreach (array_keys($keys) as $k) {
			$v = couchbase_get($h, $k);
			$this->assertEquals($v, $keys[$k]);
		}
	}
	
	/**
	 * @test
	 * Set Multi (OO)
	 *
	 * @pre
	 * connection established
	 *
	 * @post
	 * setting several items at once works
	 * 
	 * @remark
	 * Variants: OO, plain
	 */
	function testSetMultiOO() {
		$oo = $this->oo;
		$keys = array();
		for ($i = 0; $i < 3; $i++) {
			$keys[$this->mk_key()] = uniqid('couchbase-value-');
		}

		$oo->setMulti($keys);

		foreach (array_keys($keys) as $k) {
			$v = $oo->get($k);
			$this->assertEquals($v, $keys[$k]);
		}
	}
}
