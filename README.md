jansson-json 0.0.2
==================

This is a postgres extension which provides a JSON type and rudimentary
opertators and functions on them. It uses the
[Jansson JSON library](http://www.digip.org/jansson/) which you need installed
and findable by your linker.

To build semver, just do this:

    make
    make installcheck
    make install

If you encounter an error such as:

    "Makefile", line 8: Need an operator

You need to use GNU make, which may well be installed on your system as
`gmake`:

    gmake
    gmake install
    gmake installcheck

If you encounter an error such as:

    make: pg_config: Command not found

Be sure that you have `pg_config` installed and in your path. If you used a
package management system such as RPM to install PostgreSQL, be sure that the
`-devel` package is also installed. If necessary tell the build process where
to find it:

    env PG_CONFIG=/path/to/pg_config make && make installcheck && make install

If you encounter an error such as:

    ERROR:  must be owner of database regression

You need to run the test suite using a super user, such as the default
"postgres" super user:

    make installcheck PGUSER=postgres

Once jansson-json is installed, you can add it to a database. If you're running
PostgreSQL 9.1.0 or greater, it's a simple as connecting to a database as a
super user and running:

    CREATE EXTENSION "jansson-json";

If you've upgraded your cluster to PostgreSQL 9.1 and already had jansson-json
installed, you can upgrade it to a properly packaged extension with:

    CREATE EXTENSION "jansson-json" FROM unpackaged;

For versions of PostgreSQL less than 9.1.0, you'll need to run the
installation script:

    psql -d mydb -f /path/to/pgsql/share/contrib/jansson-json.sql

If you want to install jansson-json and all of its supporting objects into a
specific schema, use the `PGOPTIONS` environment variable to specify the
schema, like so:

    PGOPTIONS=--search_path=extensions psql -d mydb -f jansson-json.sql

Dependencies
------------
The `jansson-json` requires PostgreSQL and the
[Jansson JSON library](http://www.digip.org/jansson/).

Copyright and License
---------------------

This module is free software; you can redistribute it and/or modify it under
the [MIT License](http://www.opensource.org/licenses/mit-license.php).

Copyright (c) 2011 Claes Jakobsson.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
