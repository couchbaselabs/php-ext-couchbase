#!/bin/sh
exec ./runwrap.pl --exclude-group=slow $@ -c test.xml
