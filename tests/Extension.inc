<?php

require_once 'Common.inc';
/**
 *Basic extension is loadable tests, replaces 001 
 */

class Extension extends CouchbaseTestCommon
{
    /**
     * @test
     * Basic extension loading
     *
     * @pre
	 * Able to load couchbase extension
     *
     * @post
     * Extension is loaded
     */
    function testExtensionBasic() {
		$this->assertTrue(extension_loaded("couchbase"));
	}

	/**
	 * @test
	 * Extension provides Couchbase class
	 *
	 * @pre
	 * Couchbase extension is loaded
	 *
	 * @post
	 * Extension provides Couchbase class
	 */
	function testExtensionCouchbaseClassAvailable() {
		$this->assertTrue(class_exists("Couchbase"));
	}
}
?>
