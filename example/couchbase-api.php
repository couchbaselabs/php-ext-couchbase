<?php
/**
 * The content of this file is a description of the Couchbase API, so that you
 * may configure your IDE for code completion.
 */


class CouchbaseClusterManager {
    /**
     * Create a new instance of the CouchbaseClusterManager
     *
     * @param array $hosts This is an array of hostnames[:port] where the
     *                     Couchbase cluster is running. The port number is
     *                     optional (and only needed if you're using a non-
     *                     standard port).
     * @param string $user This is the username used for authentications towards
     *                     the cluster
     * @param string $password This is the password used to authenticate towards
     *                       the cluster
     */
    function __construct($hosts, $user, $password) {}

    /**
     * Get information about the cluster.
     *
     * @return string a JSON encoded string containing information of the
     *                cluster.
     */
    function getInfo() {}

    /**
     * Get information about one (or more) buckets.
     *
     * @param string $name if specified this is the name of the bucket to get
     *                     information about
     * @return string A JSON encoded string containing all information about
     *                the requested bucket(s).
     */
    function getBucketInfo($name = "") {}

    /**
     * Create a new bucket in the cluster with a given set of attributes.
     *
     * The bucket may be created with the following attributes:
     * <table border="1">
     * <tr><th>Property</th><th>Description</th></tr>
     * <tr><td>type</td><td>The type of bucket to create. This may be
     *     <code>memcached</code> or <code>couchbase</code></td></tr>
     * <tr><td>auth</td><td>The type of authentication to use to access the
     *     bucket. This may be <code>sasl</code> or <code>none</code>. If
     *  <code>none</code> is used you <b>must</b> specicy a <code>port</code>
     * attribute. for <code>sasl</code> you <b>may</b> specify a
     * <code>password</code> attribute</td></tr>
     * <tr><td>enable flush</td><td>If <code>flush()</code> should be allowed
     *     on the bucket</td></tr>
     * <tr><td>parallel compaction</td><td>If compaction of the database files
     * should be performed in parallel or not (only
     * applicable for <code>couchbase</code> buckets)</td></tr>
     * <tr><td>port</td><td>If the <code>auth</code> attribute is set to
     * <code>none</code> this attribute specifies the port number where
     * clients may access the bucket.</td></tr>
     * <tr><td>quota</td><td>This specifies the amount of memory in MB the bucket
     * should consume on <b>each</b> node in the cluster</td></tr>
     * <tr><td>index replicas</td><td>If replicas should be indexed or not (only
     * applicable for <code>couchbase</code> buckets)</td></tr>
     * <tr><td>replicas</td><td>The number of replicas to create per document.
     * The current version of Couchbase supports the following values: 0, 1, 2 and 3 (only
     * applicable for <code>couchbase</code> buckets)</td></tr>
     * <tr><td>password</td><td>This is the password used to access the bucket if
     * the <code>auth</code> attribute is set to <code>sasl</code></td></tr>
     * </table>
     *
     * @param string $name the name of the bucket to create
     * @param array $attributes a hashtable specifying the attributes for the
     *                          bucket to create.
     */
    function createBucket($name, $attributes) {}

    /**
     * Modify the attributes for a given bucket.
     *
     * Please note that you have to specify <b>all</b> attributes for the
     * bucket, so if you want to change a single attribute you should get
     * the current attributes, change the one you want and store the updated
     * attribute set.
     *
     * For a description of the different attribytes, see createBucket()
     *
     * @param string $name the name of the bucket to modify
     * @param array $attributes a hashtable specifying the new attributes for
     *                          the bucket
     */
    function modifyBucket($name, $attributes) {}

    /**
     * Delete the named bucket.
     *
     * @param string $name the bucket to delete
     */
    function deleteBucket($name) {}

}

?>
