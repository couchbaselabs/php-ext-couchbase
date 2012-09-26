<?php

if (getenv("PCBC_PHPUNIT")) {
    require_once 'PHPUnit.php';
    abstract class CBTestFramework extends PHPUnit_Framework_TestCase {

    }
} /* otherwise, the dependency is provided by the loader */

require_once 'Util.inc';

abstract class CouchbaseTestCommon extends CBTestFramework
{
    static $_handle = NULL;
    static $_oo = NULL;

    protected function setUp() {
        $this->handle = make_handle();
        $this->oo = make_handle_oo();
    }

    /**
     * Generate a new key which does not exist on the server (deleting as
     * necessary)
     *
     * @return string key
     */
    protected function mk_key() {
        $key = uniqid("couchbase_");
        $this->oo->delete($key);
        return $key;
    }

    /**
     * Returns a Couchbase resource
     */
    protected function getPersistHandle() {
        if (!self::$_handle) {
            self::$_handle = make_handle();
        }
        return self::$_handle;
    }

    /**
     * return a Couchbase handle
     */
    protected function getPersistOO() {
        if (!self::$_oo) {
            self::$_oo = make_handle_oo();
        }
        return self::$_oo;
    }

    protected function makeKvPairs($count = 10) {
        $ret = array();
        for ($ii = 0; $ii < $count; $ii++) {
            $k = $this->mk_key();
            $v = uniqid("couchbase_value_");
            $ret[$k] = $v;
        }
        return $ret;
    }

    protected function getExtVersion() {
        $version = couchbase_get_client_version();
        $version = explode('.', $version);
        for ($ii = 0; $ii < count($version); $ii++) {
            $version[$ii] = intval($version[$ii]);
        }
        return $version;
    }

    protected function atLeastVersion($varr) {
        $version = $this->getExtVersion();

        $wantval = "";
        $curval = "";

        for ($ii = 0; $ii < count($varr); $ii++) {
            $wantval .= sprintf("%02d", $varr[$ii]);
            $curval .= sprintf("%02d", $version[$ii]);
        }

        $wantval = intval($wantval);
        $curval = intval($curval);
        return ($wantval <= $curval);
    }
}

?>
