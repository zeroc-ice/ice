# Building Ice for PHP on Linux and macOS

This page describes how to build and install Ice for PHP from source code on
Linux and macOS. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## PHP Build Requirements

### Operating Systems and Compilers

Ice for PHP was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

### Ice Development Kit

You will need the Ice development kit for C++, which you can install as a binary
distribution or compile from source yourself.

## Building the PHP Extension

Our source code only supports building Ice for PHP as a dynamic PHP extension;
the product of the build is a shared library that you must configure PHP to
load.

First, change to the `php` source subdirectory:

    $ cd php

Edit `config/Make.rules` and review the build settings. For example, you may
want to enable `OPTIMIZE`. If your PHP installation resides in a non-standard
location, modify the `PHP_HOME` setting to contain the installation directory.
If you are using PHP 5.3 or later and wish to use PHP namespaces, set
`USE_NAMESPACES=yes`.

If you have not built Ice for C++ from the `cpp` subdirectory, then set the
`ICE_HOME` environment variable to the directory containing your Ice
installation. For example, if Ice is installed in `/opt/Ice`, set `ICE_HOME` as
follows:

    $ export ICE_HOME=/opt/Ice

If you installed Ice using RPM or DEB packages, set `ICE_HOME` as shown below:

    $ export ICE_HOME=/usr

Run `make` to build the extension.

## Installing the PHP Extension

To install the Ice extension, you must move the extension's shared library into
PHP's extension directory. This directory is determined by the PHP configuration
directive `extension_dir`. You can determine the default value for this
directive by running the command-line version of PHP with the `-i` option:

    $ php -i

Review the output for a line like this:

    extension_dir => /usr/lib/php/modules => /usr/lib/php/modules

Once you've copied the extension to PHP's extension directory, you will need to
enable the extension in your PHP configuration. Your PHP installation likely
supports the `/etc/php.d` configuration directory, which you can verify by
examining the output of `php -i` and looking for the presence of
`--with-config-file-scan-dir` in the "Configure Command" entry. If present,
you can create a file in `/etc/php.d` that contains the directive to load the
Ice extension. For example, create the file `/etc/php.d/ice.ini` containing
the following line on Linux:

    extension = IcePHP.so

Or on macOS:

    extension = IcePHP.dy

If PHP does not support the `/etc/php.d` directory, determine the path name of
PHP's configuration file as reported by the `php -i` command:

    Configuration File (php.ini) Path => /etc/php.ini

Open the configuration file and append the following line on Linux:

    extension = IcePHP.so

Or on macOS:

    extension = IcePHP.dy

You can verify that PHP is loading the Ice extension by running the command
shown below:

    $ php -m

Look for `ice` among the installed modules. Note that your library search path
(`LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS) must include the
directory containing the Ice shared libraries.

Read the PHP Dependencies and PHP Source Files sections below for more
information about installing the Ice extension.

## PHP Dependencies

PHP will need to be able to locate the libraries for the Ice run-time libraries
and its third-party dependencies. These libraries are named as follows:

    libIce
    libIceUtil
    libSlice
    libbz2

In general, these libraries must reside in a directory of the user's PATH. For
Web servers, the libraries may need to reside in a system directory. For
example, on Linux you can add the directory containing the Ice run-time
libraries to `/etc/ld.so.conf` and run `ldconfig`.

You can verify that the Ice extension is installed properly by examining the
output of the `php -m` command, or by calling the `phpInfo()` function from a
script. For example, you can create a file in the Web server's document
directory containing the following PHP script:

    <?php
    phpInfo();
    ?>

Then start a browser window and open the URL corresponding to this script. If
the Ice extension is successfully installed, you will see an `ice` section among
the configuration information.

Note that if you want to use IceSSL from the Ice extension, then PHP will also
need access to the shared libraries for IceSSL and OpenSSL.

## PHP Source Files

In addition to the binary Ice extension module and its library dependencies, you
will also need to make the Ice for PHP source files available to your scripts.
These files are located in the `lib` subdirectory and consist of the Ice run
time definitions (`Ice.php` or `Ice_ns.php`) along with PHP source files
generated from the Slice files included in the Ice distribution.

The Ice extension makes no assumptions about the location of these files, so you
can install them anywhere you like. For example, you can simply include them in
the same directory as your application scripts. Alternatively, if you prefer to
install them in a common directory, you may need to modify PHP's `include_path`
directive so that the PHP interpreter is able to locate these files. Another
option is to modify the include path from within your script prior to including
any Ice run-time file. Here is an example that assumes Ice is installed in
`/opt`:

    // PHP
    ini_set('include_path',
    ini_get('include_path') . PATH_SEPARATOR . '/opt/Ice/php');
    require 'Ice.php'; // Load the core Ice run time definitions.

## Running the PHP Tests

The test subdirectory contains PHP implementations of the core Ice test suite.
Python is required to run the test suite.

The test suites require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution. In addition, the scripts require
that the CLI version of the PHP interpreter be available in your PATH.

After a successful build, you can run the tests as follows:

    $ python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Web Server Permissions

The Web server normally runs in a special user account that may not necessarily
have access to the Ice extension, its dependent libraries and PHP source files,
and other resources such as Ice configuration and your application scripts. It
is very important that you review the permissions of these files and verify
that the Web server has sufficient access.

On Linux, Apache typically runs in the `apache` account, so you will either
need to change the owner or group of the libraries and other resources, or
modify their permissions to make them sufficiently accessible.

## SELinux Notes for PHP

SELinux augments the traditional Unix permissions with a number of new features.
In particular, SELinux can prevent the httpd daemon from opening network
connections and reading files without the proper SELinux types.

If you suspect that your Ice for PHP application does not work due to SELinux
restrictions, we recommend that you first try it with SELinux disabled.
As root, run:

    # setenforce 0

to disable SELinux until the next reboot of your computer.

If you want to run httpd with Ice for PHP and SELinux enabled, there are two
steps you need to take. First, allow httpd to open network connections:

    # setsebool httpd_can_network_connect=1

Add the `-P` option to make this setting persistent across reboots.

Second, make sure any `.ice` file used by your PHP scripts can be read by httpd.
The enclosing directory also needs to be accessible. For example:

    # chcon -R -t httpd_sys_content_t /opt/MyApp/slice

[1]: https://zeroc.com/downloads/ice
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+and+Ice+Touch+3.6.5
