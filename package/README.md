# Couchbase driver for PHP

Contents:

  ./
  ./README.md       This file.
  ./couchbase.so    The PHP extension as a shared library.


## Installation

Edit your `php.ini` to include this line:

    extension=couchbase.so

To find where you `php.ini` file is, try `phpi -i | grep couchbase` or look
at your `<?php phpinfo() ?>` output.


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
