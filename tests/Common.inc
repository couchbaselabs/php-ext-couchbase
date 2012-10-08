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

    function skipALL() {

    }

    protected function setUp() {
        $this->setErrorsRaiseException(true);
        $this->handle = make_handle();
        $this->oo = make_handle_oo();
        $this->skipALL();
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

    /**
     * Make all warnings raise an error exception. This is the
     * default, but might help to do so explicitly.
     *
     * @param $b boolean, whether to raise an ErrorException for the
     * error handler
     *
     * @return the old error handler
     */
    protected function setErrorsRaiseException($b = true) {
        $ret = NULL;
        if ($b) {
            $ret = set_error_handler(
                function ($errno,$errstr,$errfile,$errline,$errctx) {
                    throw new ErrorException(
                        $errstr, $errno, $errno, $errfile, $errline);
                });
        } else {
            $ret = set_error_handler(function() { });
        }
        return $ret;
    }

    /**
     * Invoke a function in a 'safe' context and capture any fatals
     *
     * @param func the function to call
     * @param args the arguments to pass to the function
     * @param expr a regular expression the output should match
     */
    protected function
    assertFatalRegexp($func, $args, $expr, $maxwait = 10, $msg = "") {

        $tmpfile = tempnam("", "php_couchbase_tests-");
        $pid = pcntl_fork();

        if (!$pid) {
            fclose(STDOUT);
            global $STDOUT;
            global $STDERR;

            $STDERR = fopen($tmpfile, "a");
            $STDOUT = STDERR;

            pcntl_alarm($maxwait);
            call_user_func_array($func, $args);

            exit(0);
        }

        pcntl_waitpid($pid, $status);
        $lines = file($tmpfile);
		die("line count: ".count($lines));
        $txt = join(" ", $lines);

        $found_match = preg_match($expr, $txt) > 0;
        $this->assertRegExp($expr, $txt, $msg);
        return true;
    }

    /**
     * Invoke a function in a 'safe' context and assert that the exception's
     * message matches the regular expression
     *
     * @param func the function to invoke
     * @param args the arguments to pass
     * @param expr a regular expression the output should match
     */
    protected function
    assertExceptionRegexp($func, $args, $expr, $msg = "") {
        $errmsg = NULL;
        try {
            call_user_func_array($func, $args);
        } catch (Exception $exc) {
            $errmsg = $exc->getMessage();
        }

        $this->assertNotNull($errmsg);
        $this->assertRegExp($expr, $errmsg, $msg);
        return true;
    }
}

?>
