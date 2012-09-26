<?php

require_once 'Common.inc';
require_once 'GetMulti.php';

class BigValues extends CouchbaseTestCommon {
    /**
     * @group slow
     */
    function testBigValue() {
        $k = $this->mk_key();
        $v = str_repeat("*", 0x1000000);
        $oo = $this->getPersistOO();
        $rv = $oo->set($k, $v);
        $this->assertNotEmpty($rv);

        $ret = $oo->get($k);
        $this->assertEquals($ret, $v);

        // Delete it, don't leave crap on the server
        $oo->delete($k);
    }
}
