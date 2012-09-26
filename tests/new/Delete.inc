<?php
require_once 'Common.inc';
class Delete extends CouchbaseTestCommon {

        /* superseds 008 */

    /**
     * @test
     * Delete
     *
     * @pre Add a key and delete it.
     * @post Delete succeeds. Subsequent GET returns @c KEY_ENOENT
     *
     * @remark
     * Variants: OO
     *
     * @test
     * Delete (with CAS)
     *
     * @pre Add a key, modify its value. Store the cas in each operation as an
     * array of @c $cas. Delete using @c $cas[0], and then delete using
     * @c $cas[1]
     *
     *
     * @post
     * Delete with the outdated @c $cas[0] fails, Delete with @c $cas[1]
     * Succeeds
     *
     * @remark
     * Variants: OO
     *
     * @test_plans{7.1, 7.3, 7.4}
     */
    function testDeleteOO() {
        $key = $this->mk_key();
        $value = uniqid("couchbase_");
        $oo = $this->oo;

        $oo->add($key, $value);
        $this->assertNotEmpty($oo->get($key));
        $oo->delete($key);
        $this->assertNull($oo->get($key));

        # Test CAS
        $cas = $oo->add($key, $value);
        $cas2 = $oo->set($key, "bar");

        $this->assertNotEmpty($cas2);

        $this->assertFalse($oo->delete($key, $cas),
                           "delete Fails on stale cas");

        $this->assertEquals($oo->get($key), "bar");

        $oo->delete($key, $cas2);
        $this->assertNull($oo->get($key));
    }

    function testDelete() {
        $key = $this->mk_key();
        $value = uniqid("couchbase_");
        $h = $this->handle;
        couchbase_add($h, $key, $value);
        $this->assertNotEmpty(couchbase_get($h, $key));
        couchbase_delete($h, $key);
        $this->assertNull(couchbase_get($h, $key));

        $cas = couchbase_add($h, $key, $value);
        $cas2 = couchbase_set($h, $key, "bar");
        $this->assertNotEmpty($cas2);
        $this->assertFalse(couchbase_delete($h, $key, $cas));
        $this->assertEquals("bar", couchbase_get($h, $key));

        couchbase_delete($h,$key,$cas2);
        $this->assertNull(couchbase_get($h, $key));
    }

    /**
     * @test Delete (non-exist)
     * @pre Store a key, delete it once, delete it again
     * @post First delete should return a value which is not false, second
     * delete should return false
     *
     * @test_plans{7.2}
     */
    function testDeleteNonExist() {
        $oo = $this->getPersistOO();
        $k = $this->mk_key();
        $this->assertNotEmpty($oo->set($k, 'me3h'));

        $this->assertTrue((bool)$oo->delete($k));
        $this->assertFalse($oo->delete($k));
    }

}
