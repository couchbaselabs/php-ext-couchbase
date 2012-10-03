<?php
require_once 'Common.inc';

class Replace extends CouchbaseTestCommon {

    # 013

    /**
     * @test Replace
     *
     * @pre
     * Use replace to modify a non existent key.
     * Set the key (via set), then replace it again
     *
     * @post
     * First replace fails.
     * Second replace succeeds (with valid CAS return value)
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{2.5, 2.4}
     */
    function testReplaceOO() {
        $oo = $this->oo;
        $key = $this->mk_key();

        $rv = $oo->replace($key, "bar");
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            $oo->getResultCode());

        $oo->set($key, "foo");
        $rv = $oo->replace($key, "bar");
        $this->assertNotEmpty($rv);
        $val = $oo->get($key);
        $this->assertEquals("bar", $val);
    }

    function testReplace() {
        $h = $this->handle;
        $key = $this->mk_key();

        $rv = couchbase_replace($h, $key, "bar");
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            couchbase_get_result_code($h));

        couchbase_set($h, $key, "foo");
        $rv = couchbase_replace($h, $key, "bar");
        $this->assertNotEmpty($rv);

        $val = couchbase_get($h, $key);
        $this->assertEquals("bar", $val);
    }

    /**
     * @test
     * @pre Set a key, get its CAS, replace it using the cas received
     * @post Replace succeeds. Get on the key now yields the value passed to
     *  @c replace
     *
     * @test_plans{2.8.3}
     */
    function testReplaceCas() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $v = "a value..";

        $cas = $oo->set($k, $v);
        $this->assertNotEmpty($cas);

        $v = "second value..";
        $rv = $oo->replace($k, $v, 0, $cas);
        $this->assertNotEmpty($rv);

        $ret = $oo->get($k, NULL, $cas2);
        $this->assertEquals($v, $ret);
		$this->assertNotEquals($cas, $cas2);
    }

    /**
     * @test
     * @pre set a key, then replace it with a garbage cas ( @c 1234567 )
     * @post Replace fails, getting the key yields the original value
     * @test_plans{2.8.4}
     */
    function testReplaceInvalidCas() {
        $k = $this->mk_key();
        $v = "a value";
        $oo = $this->getPersistOO();

        $cas = $oo->set($k, $v);

        $rv = $oo->replace($k, "shouldn't show", 0, 1);

        $this->assertFalse($rv);

        $ret = $oo->get($k);
        $this->assertEquals($v, $ret);
    }
}
