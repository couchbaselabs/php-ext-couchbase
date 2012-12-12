# PHP PECL extension for Couchbase Server

Contents:

  ./
  ./README.md       This file.
  ./couchbase.so    The PHP extension as a shared library.


## Installation

Edit your `php.ini` to include this line after putting the extension
in the correct directory.:

    extension=couchbase.so

If you're on a Red Hat Enterprise Linux or CentOS platform, this is
best done as:

    extension=json.so
    extension=couchbase.so

Note that in either case you can use a complete path, such as:

    extension=/path/to/couchbase.so

To find where your `php.ini` file is, try `php --ini` or
look at your `<?php phpinfo() ?>` output. This can also identify the
extension directory if you wish to copy the extension there.


## Verify Installation

Run this test script:

    <?php
    $cb = new Couchbase("127.0.0.1:8091", "username", "password", "default");
    $cb->set("a", 1);
    var_dump($cb->get("a"));

Make sure you have started your Couchbase server. Adjust the hostname,
port, user name and password as needed.


## Support

If you have an questions, problems or suggestions, please let us know
on the Couchbase SDK forums:

http://www.couchbase.org/forums/sdks/sdks

## License

Apache License 2.0 — See LICENSE for details.
