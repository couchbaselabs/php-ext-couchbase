<?php

require_once 'Common.inc';

/* replaces 018 */

class EmptyKey extends CouchbaseTestCommon
{
    /**
     * @param $fn The couchbase function to execute
     * @param $params The params to pass to the function
     * @param $do_skip Whether to not run the assertion. Needed for the
     * *_multi functions.
     */
    private function _empty_key_common($fn, $params, $do_skip = false) {
        $msg = NULL;
        $oo = $this->oo;

        if ($do_skip) {
            $this->markTestSkipped("Skipping empty key test for " . $fn);
        }

        try {
            call_user_func_array( array(&$oo, $fn), $params);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
            $this->assertContains('Empty', $msg,
                                  "Got 'Empty' exception for " . $fn);
        }
        $this->assertNotNull($msg, 'Got an exception for ' . $fn);
    }

    /**
     * @test Empty Keys
     *
     * @pre
     *
     * perform a mutation (the operation is passed as an argument to
     * this function) operation with an empty key
     *
     * @post
     * Error message indicating that an empty key was passed
     *
     * @test_plans{2.1.3, 3.5}
     *
     * @remark
     * Variants: Set, Add, Replace
     */
    public function testEmptyKeySet() {
        $this->_empty_key_common('set', array('', 'foo'));
    }

    public function testEmptyKeyReplace() {
        $this->_empty_key_common('replace', array('', 'foo'));
    }

    public function testEmptyKeyAdd() {
        $this->_empty_key_common('add', array('', 'foo'));
    }

    public function testEmptyKeyGet() {
        $this->_empty_key_common('get', array(''), true);
    }

    public function testEmptyKeyGetMulti() {
        $this->_empty_key_common('getMulti', array(array("")), true);
    }

    public function testEmptyKeySetMulti() {
        $this->_empty_key_common('setMulti', array(array("")), true);
    }

}

?>
