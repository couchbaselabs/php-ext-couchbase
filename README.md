# ext/couchbase

A PHP extension wrapping libcouchbase.

## Dependencies

libcouchbase
igbinary (optional)

## Installation

End users should install the module by executing:

    $ pecl install couchbase

Developers who want to build the latest bits from the source
repository may execute the following steps

    $ phpize
    $ ./configure
    $ make
    $ make test

Running make test assumes that you have a Couchbase server listening
on port 127.0.0.1:8091

If you are using a webserver installation of PHP, you might need to
restart your webserver.

If you have igbinary installed, make test will most likely fail with
an error message reporing a missing symbol. Don't worry; it has an
explanation. During make test php generates another php.ini file for
us which don't include the igbinary module. To work around this all
you need to do is to edit the Makefile and add the following as the
first parameter parameter to run-tests.php:

    -d extension=/absolute/path/to/igbinary.so

## First Step

Create a new script `test.php`:

    <?php
    $cb = new Couchbase("127.0.0.1:8091", "Administrator",
                        "password", "default");
    $cb->set("a", 1);
    $a = $cb->get("a");
    echo $a;
    ?>

# Compatibility

PHP 5.3 or later.

Windows support is coming in the next release.

## License

Apache License 2.0

## Credits

See CREDITS file. Thanks all!

## Copyright

(c) 2011, 2012, 2013 Couchbase

## Contributions

If you'd like to contribute to the project, please look in
CONTRIBUTING.md.
