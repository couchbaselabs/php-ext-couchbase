<?php

require_once 'Common.inc';

class Unlock extends CouchbaseTestCommon {

    function testUnlockOO() {
        $oo = $this->oo;
        $key = $this->mk_key();
        $cas = $oo->set($key, "foobar");

        # Its time to get the lock of the object
        $valGet = $oo->getAndLock($key, $lockCAS);

        # validate that I can't overwrite it
        try {
            $oo->set($key, "should fail");
            $this->assertFalse(true);
        } catch (CouchbaseException $e) {
            $this->assertEquals(COUCHBASE_KEY_EEXISTS, $oo->getResultCode());
        }

        # Unlock the thing and try to overwrite it. this time it should work
        $this->assertTrue($oo->unlock($key, $lockCAS));
        $cas2 = $oo->set($key, "should succeed");
        $this->assertTrue(is_string($cas2));
    }

    function testUnlock() {
        $h = $this->handle;
        $key = $this->mk_key();
        $cas = couchbase_set($h, $key, "foobar");

        # Its time to get the lock of the object
        $valGet = couchbase_get_and_lock($h, $key, $lockCAS);

        # validate that I can't overwrite it
        try {
            couchbase_set($h, $key, "should fail");
            $this->assertFalse(true);
        } catch (ErrorException $e) {
            $this->assertEquals(COUCHBASE_KEY_EEXISTS,
                                couchbase_get_result_code($h));
        }

        # Unlock the thing and try to overwrite it. this time it should work
        $this->assertTrue(couchbase_unlock($h, $key, $lockCAS));
        $cas2 = couchbase_set($h, $key, "should succeed");
        $this->assertTrue(is_string($cas2));
    }
}
