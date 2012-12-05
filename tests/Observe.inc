<?php
require_once 'Common.inc';

class Observe extends CouchbaseTestCommon {
	/* this relies on observe support, introduced in 1.1 */
	function skipALL() {
		parent::skipALL();
		if (!$this->atLeastVersion(array(1,1,0))) {
			$this->markTestSkipped("need 1.1.x for observe support");
		}
	}

    /**
     * Gets us a durability spec appropriate for the cluster we're running on
     */
    function _make_max_spec() {
        $spec = array();
        $h = $this->getPersistHandle();

        /* The bucket may be configured with more replicas than
         * we've got nodes...
         */
        $num_servers = count(couchbase_get_servers($h));
        if ($num_servers <= couchbase_get_num_replicas($h)) {
            $spec["persist_to"] = $num_servers;
            $spec["replicate_to"] = $num_servers - 1;
        } else {
            $spec["persist_to"] = couchbase_get_num_replicas($h) + 1;
            $spec["replicate_to"] = couchbase_get_num_replicas($h);
        }
        return $spec;
    }

	/*	@test basic observe test
	 *
	 *  @pre
	 *  connects, sets key to value
	 *
	 *  @post
	 *  observe for snapshot view
	 *
	 *  @remark
	 *  Variants: OO, plain
	 */
	function testBasicObserve() {
        $h = $this->getPersistHandle();

		$kvpair = $this->makeKvPairs(1);
		list($k) = array_keys($kvpair);
		$v = $kvpair[$k];

		$cas = couchbase_set($h, $k, $v);
        sleep(1);

        $this->assertNotEmpty($cas);

		$rv = couchbase_observe($h, $k, $cas, $details);

		$this->assertTrue($rv);
		$this->assertArrayHasKey("responses", $details[$k]);
		$this->assertArrayHasKey("ttr", $details[$k]);
		$this->assertArrayHasKey("ttp", $details[$k]);
		$this->assertArrayHasKey("error", $details[$k]);
		/* TODO: add some checks on details values */
	}

	function testBasicObserveOO() {
        $oo = $this->getPersistOO();

		$kvpair = $this->makeKvPairs(1);
		list($k) = array_keys($kvpair);
		$v = $kvpair[$k];
		$cas = $oo->set($k, $v);

		$rv = $oo->observe($k, $cas, $details);

		$this->assertTrue($rv);
		$this->assertArrayHasKey("responses", $details[$k]);
		$this->assertArrayHasKey("ttr", $details[$k]);
		$this->assertArrayHasKey("ttp", $details[$k]);
		$this->assertArrayHasKey("error", $details[$k]);

		/* TODO: add some checks on details values */
	}

    /**
     * @test Observe the key modified with a different CAS
     * @pre Create 10 kv pairs and set them. Store the old CAS. Store them again,
     * this time getting the new CAS.
     *
     * @post Run an observe function with details. The error string for each
     * of the keys should be 'CAS Mismatch' and the boolean return array should
     * be false for all of the keys
     *
     * @todo The API should really allow us to see the new CAS?
     */
    function testModified() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();

        $cas1 = $oo->set($k, "first_value");
        $cas2 = $oo->set($k, "second_value");

        $rv = $oo->observe($k, $cas1, $details);

        $this->assertFalse($rv);
        $this->assertArrayHasKey($k, $details);

        $kd = $details[$k];
        $this->assertArrayHasKey('error', $kd);
        $this->assertEquals('CAS Mismatch', $kd['error']);
        $this->assertEquals(COUCHBASE_KEY_EEXISTS, $kd['error_code']);
    }

    /**
     * @test Various bad parameters for observe. In all these cases
     * we should get appropriate error messages and have the functions return
     * accordingly.
     *
     * @pre Test with an invalid (non-numeric CAS)
     * @pre Test with an invalid timeout value
     * @pre Test with a negative timeout value
     *
     * @todo Add more here
     */
    function testBadParams() {
        $oo = $this->getPersistOO();
        $this->assertExceptionRegexp(
            create_function('$oo,$kv', '$oo->observe($kv, "blahblah", $details);'),
            array($oo, "some_key"),
            "/Invalid CAS/"
        );

        $spec = $this->_make_max_spec();
        $spec['timeout'] = 'blahblaha';

        $this->assertExceptionRegexp(
            create_function('$oo, $spec',
                            '$oo->keyDurability("akey", "123", $spec);'),
            array($oo, $spec),
            "/must be numeric/i"
        );


        $spec['timeout'] = -1;
        $this->assertExceptionRegexp(
            create_function('$oo, $spec',
                            '$oo->keyDurability("akey", "123", $spec);'),
            array($oo, $spec),
            "/must be greater/i"
        );
    }

    /**
     * @test Polling behavior on empty keys
     * @pre Pass an empty array to @c keyDurabilityMulti
     * @post Function returns
     */
    function testMultiEmpty() {
        $oo = $this->getPersistOO();
        $rv = $oo->keyDurabilityMulti(array(), $this->_make_max_spec());
    }

    /**
     * @test Snapshot behavior on a NULL key
     * @pre Pass a NULL key (with a NULL cas) to the observe function
     * @post Function returns
     */
    function testSnapshotEmpty() {
        $oo = $this->getPersistOO();
        $rv = $oo->observe(NULL, NULL, $details);
    }

    /**
     * @test Specify criteria that cannot be met
     * @pre Create a 'durspec' with a persist_to of 100 and a replicate_to of 100
     * @post Run the keyDurability command. A warning should appear saying that
     * the criteria have been scaled down per the topology of the cluster
     *
     * @todo differentiate between "less" persist and replicate to "no" persist
     * and replicate
     */
    function testPollingExcessive() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $cas = $oo->set($k, "some value");

        $errtxt = "";

        $this->setCaptureError($errtxt);

        $spec = array("persist_to" => 100, "replicate_to" => 100);
        $rv = $oo->keyDurability($k, $cas, $spec);
        $this->restoreErrorHandler();

        $this->assertContains('Capping', $errtxt);
    }

    /**
     * @test Test mixed multi polling
     *
     * @pre Make three kv sets, one is @c $kv_ok for which we have a valid CAS
     * one is @c $kv_stale for which the key exists on the cluster, but we have
     * an invalid cas, and finally @c $kv_noexist for keys that do not exist
     * on the cluster
     *
     * @post Mix them all together in a key:cas array (including the bogus CAS
     * values) and run the keyDurabilityMulti function. The function should
     * not fail. The keys for which a valid CAS were provided should show up
     * as true in the boolean return value. The others should appear as false
     */
    function testPollingMultiMixed() {
        $oo = $this->getPersistOO();

        $kv_ok = $this->makeKvPairs(10);
        $kv_stale = $this->makeKvPairs(10);
        $kv_noexist = $this->makeKvPairs(10);

        $casarray = array();

        foreach ($kv_ok as $k => $v) {
            $casarray[$k] = $oo->set($k, "a value");
        }
        foreach ($kv_stale as $k => $v) {
            $casarray[$k] = "10101010101";
            $oo->set($k, "a value");
        }
        foreach($kv_noexist as $k => $v) {
            $casarray[$k] = '666666666666';
        }

        $h = $this->getPersistHandle();
        $spec = $this->_make_max_spec();
        $rvs = $oo->keyDurabilityMulti($casarray, $spec);

        foreach ($kv_ok as $k => $v) {
            $this->assertArrayHasKey($k, $rvs);
            $this->assertTrue($rvs[$k]);
        }

        foreach ($kv_stale as $k => $v) {
            $this->assertArrayHasKey($k, $rvs);
            $this->assertFalse($rvs[$k]);
        }

        foreach ($kv_stale as $k => $v) {
            $this->assertArrayHasKey($k, $rvs);
            $this->assertFalse($rvs[$k]);
        }
    }

    /**
     * @test Timeout behavior
     *
     * @pre Run a polling command against a non-existent key - this will
     * have the client loop until the timeout expires
     *
     * @post Measure the duration time. It should be no greater than twice the
     * timeout
     *
     * @post Assert that a default timeout does exist
     */
    function testTimeout() {
        $oo = $this->getPersistOO();
        # Get ourselves a key that does not exist..
        $key = $this->mk_key();
        $oo = $this->getPersistOO();

        $t_begin = time();

        $rv = $oo->keyDurability($key, "1234", $this->_make_max_spec());

        $t_end = time();

        $duration = $t_end - $t_begin;
        $max = ini_get("couchbase.durability_default_timeout");
        $this->assertTrue($max > 0);
        $this->assertTrue($duration < (($max * 2) / 1000000));

        $this->assertFalse($rv);
    }

	/* @test multi-key basic observe test
	 *
	 * @pre
	 * connects, sets a set of keys to values
	 *
	 * @post
	 * observe for snapshot view of those keys
	 *
	 * @remark
	 * Variants: OO, plain
	 */
	function testBasicMultiObserve() {
        $h = $this->getPersistHandle();

		$kvpairs = $this->makeKvPairs(10);
		$kc = array();

		foreach ($kvpairs as $k=>$v) {
			$kc[$k] = couchbase_set($h, $k, $v);
		}

        sleep(2);

		$rv = couchbase_observe_multi($h, $kc, $details);

		foreach ($rv as $k=>$krv) {
			$this->assertTrue($krv);
			$this->assertArrayHasKey("responses", $details[$k]);
			$this->assertArrayHasKey("ttr", $details[$k]);
			$this->assertArrayHasKey("ttp", $details[$k]);
			$this->assertArrayHasKey("error", $details[$k]);
		}
	}

	function testBasicMultiObserveOO() {
        $oo = $this->getPersistOO();

		$kvpairs = $this->makeKvPairs(10);
		$kc = array();

		foreach ($kvpairs as $k=>$v) {
			$kc[$k] = $oo->set($k, $v);
		}

		$rv = $oo->observeMulti($kc, $details);

		foreach ($rv as $k=>$krv) {
			$this->assertTrue($krv);
			$this->assertArrayHasKey("responses", $details[$k]);
			$this->assertArrayHasKey("ttr", $details[$k]);
			$this->assertArrayHasKey("ttp", $details[$k]);
			$this->assertArrayHasKey("error", $details[$k]);
		}
	}

	/* @test single-key, simple durability requirements
	 *
	 * @pre
	 * connects, sets a key to a value
	 *
	 * @post
	 * key meets given (basic) durability requirements
	 *
	 * @group
	 * slow
	 *
	 * @remark
	 * Variants: OO, plain
	 */
	function testKeyDurability() {
        $h = $this->getPersistHandle();

		$kvpairs = $this->makeKvPairs(1);
		list($k) = array_keys($kvpairs);
		$v = $kvpairs[$k];

		$cas = couchbase_set($h, $k, $v);

		$durability = $this->_make_max_spec();

		$rv = couchbase_key_durability($h, $k, $cas, $durability);

		$this->assertTrue($rv);
	}

	function testKeyDurabilityOO() {
        $oo = $this->getPersistOO();

		$kvpairs = $this->makeKvPairs(1);
		list($k) = array_keys($kvpairs);
		$v = $kvpairs[$k];

		$cas = $oo->set($k, $v);

		$durability = array('persist_to' => 1, 'replicate_to' => 0);
		$rv = $oo->keyDurability($k, $cas, $durability);

		$this->assertTrue($rv);
	}
}

?>
