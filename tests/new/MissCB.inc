<?php
require_once 'Common.inc';

/* 005 */

/**
 * A GET miss callback which populates the passed value pointer and
 * return trues
 */
function cache_cb($res, $key, &$value) {
    $value = "from_db";
    return true;
}

/**
 * a GET miss callback which functions like @ref cache_cb, but
 * returns false
 */
function another_cache_cb($res, $key, &$value) {
    $value = "from_db";
    return false;
}

class MissCB extends CouchbaseTestCommon {

    /**
     * @test Miss Callback (with Hit)
     *
     * @pre Create a callback which sets the value pointer to
     * "from_db", and returns true.
     * Set a key to a value "foo", and perform a get on the key,
     * specifying the miss callback
     *
     * @post Value is "foo", CAS is non-empty.
     *
     * @remark
     * Variants: OO
     *
     * @test Miss Callback (with Miss)
     *
     * @pre
     * Create a callback as in the hit test, get a
     * (non-existent) key, passing the callback to get
     *
     * @post
     * Value is "from_db", CAS is empty
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{3.2}
     */
    function testMissCbOO() {
        $key = $this->mk_key();
        $this->oo->set($key, "foo");
        $rv = $this->oo->get($key, "cache_cb", $cas);
        $this->assertNotEmpty($cas);
        $this->assertEquals('foo', $rv);

        $this->oo->delete($key);
        $rv = $this->oo->get($key, "cache_cb", $cas);
        $this->assertNull($cas);
        $this->assertEquals("from_db", $rv);

        $rv = $this->oo->get($key, "another_cache_cb", $cas);
        $this->assertNull($cas);
        $this->assertNull($rv,
                "Return value is empty because function returned false");
    }

    function testMissCb() {
        $key = $this->mk_key();
        $h = $this->handle;
        couchbase_set($h, $key, "foo");
        $rv = couchbase_get($h, $key, "cache_cb", $cas);
        $this->assertNotEmpty($cas);
        $this->assertEquals("foo", $rv, "Got value from cluster and not CB");

        couchbase_delete($h, $key);
        $rv = couchbase_get($h, $key, "cache_cb", $cas);
        $this->assertNull($cas, "Cas not returned from callback");
        $this->assertEquals("from_db", $rv);

        $rv = couchbase_get($h, $key, "another_cache_cb", $cas);
        $this->assertNull($cas);
        $this->assertNull($rv);
    }
}
