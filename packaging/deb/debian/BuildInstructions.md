# Building DEB Packages

This document describes how to create source and binary DEB packages for Ice as
a regular (non-root) user on your Debian or Ubuntu Linux system.

## Setup the packaging software

First we'll install all the tools needed to build the Ice packages on your Ubuntu
system:

    $ sudo apt-get install packaging-dev git-buildpackage

## Checkout Ice source

    $ git clone git@github.com:zeroc-ice/ice-debian-packaging
    $ git checkout -b upstream --track origin/upstream
    $ git checkout master

## Install Ice third-party dependencies

Now we will install the necessary third-party packages from the system Software
repository.

    $ sudo mk-build-deps --install ice-debian-packaging/debian/control

## Building the Ice packages


    $ cd ice-debian-packaging
    $ gbp buildpackage

## Applying a patch

Refer to the [Debian documentation][1] for information on incorporating patches
into the build.

[1]: https://www.debian.org/doc/manuals/maint-guide/dother.en.html#patches
