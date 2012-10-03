<?php

/* 016, 033 */

require_once 'Common.inc';
class Errors extends CouchbaseTestCommon {

    /**
     * @test Error Reporting
     * @pre Perform various actions and see their error messages. Each action
     * will inspect the get_result_code value as well as the
     * get_result_message value
     *
     * @post
     * get-non-exist: KEY_ENOENT/'No Such Key'.
     * replace-non-exist: KEY_ENOENT.
     * add-key: SUCCESS/'Success',
     * delete-key: SUCCESS/'Success'.
     * append-non-existent: NOT_STORED/'Not stored'.
     * prepend-non-existent: NOT_STORED/'Not stored'.
     * bad-cas: KEY_EEXISTS/'Key exists (with a different CAS value)'
     */
    function testErrorCodes() {
        $key = $this->mk_key();
        $h = $this->handle;
        $handle = $h; // alias

        // really make sure it doesn't exist..
        $this->oo->delete($key);

        $rv = couchbase_get($h, $key);
        $this->assertNull($rv);

        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            couchbase_get_result_code($h));

        $this->assertEquals('No such key',
                            couchbase_get_result_message($h));


        $rv = couchbase_replace($h, $key, 'foo');
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_KEY_ENOENT,
                            couchbase_get_result_code($handle));

        $rv = couchbase_add($h, $key, 'foo');
        $this->assertEquals(COUCHBASE_SUCCESS,
                            couchbase_get_result_code($h));
        $this->assertEquals('Success',
                            couchbase_get_result_message($h));

        $rv = couchbase_delete($h, $key);
        $this->assertEquals(COUCHBASE_SUCCESS,
                            couchbase_get_result_code($handle));

        $rv = couchbase_append($h, $key, 'append');
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_NOT_STORED,
                            couchbase_get_result_code($handle));
        $this->assertEquals('Not stored',
                            couchbase_get_result_message($h));

        $rv = couchbase_prepend($h, $key, 'prepend');
        $this->assertFalse($rv);
        $this->assertEquals(COUCHBASE_NOT_STORED,
                            couchbase_get_result_code($h));

        // Cas results..
        $key = $this->mk_key();
        $cas = couchbase_set($h, $key, "value");
        couchbase_set($h, $key, "value");
        $rv = couchbase_cas($h, $cas, $key, "other_value");
        $this->assertEquals(COUCHBASE_KEY_EEXISTS,
                            couchbase_get_result_code($h));
        $this->assertEquals('Key exists (with a different CAS value)',
                            couchbase_get_result_message($h));
    }
}

?>
