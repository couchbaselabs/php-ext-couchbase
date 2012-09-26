<?php
require_once 'Common.inc';

class MiscOptions extends CouchbaseTestCommon {
    /**
     * @test Invalid Options
     * @pre set an invalid option @c 1111 to an invalid value @c 111
     * @post Error is thrown
     * @remark
     * Variants: OO
     */
    function testMiscOptions() {
        $h = $this->getPersistHandle();
        $oo = $this->getPersistOO();

        $msg = NULL;
        try {
            $oo->setOption(1111, 111);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('unknown option', $msg);

        $msg = NULL;
        try {
            couchbase_set_option($h, 1111, 111);
        } catch (Exception $exc) {
            $msg = $exc->getMessage();
        }
        $this->assertNotNull($msg);
        $this->assertContains('unknown option', $msg);
    }
}
