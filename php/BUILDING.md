# Ice for PHP Build Instructions

This page describes how to build and install Ice for PHP from source.

ZeroC provides [binary distributions] for many platforms, including Linux, so building Ice for PHP from source is
usually unnecessary.

- [PHP Build Requirements](#php-build-requirements)
- [Building the PHP Extension](#building-the-php-extension)
- [Installing the PHP Extension](#installing-the-php-extension)
- [PHP Dependencies](#php-dependencies)
- [PHP Source Files](#php-source-files)
- [Running the PHP Tests](#running-the-php-tests)

## PHP Build Requirements

Ice was extensively tested using the operating systems and compiler versions listed on [supported platforms].

## Building the PHP Extension

The build of Ice for PHP requires that you first build Ice for C++ in the `cpp` subdirectory.

From the top-level source directory, edit `config/Make.rules` to establish your build configuration. The comments in
the file provide more information.

Our source code only supports building Ice for PHP as a dynamic PHP extension; the product of the build is a shared
library that you must configure PHP to load.

Change to the `php` source subdirectory:

```shell
cd php
```

Ensure that `php` and `php-config` for the version of PHP you wish to build against are first in your PATH.

Run `make` to build the extension.

## Installing the PHP Extension

To install the Ice extension, you must move the extension's shared library into PHP's extension directory. This
directory is determined by the PHP configuration directive `extension_dir`. You can determine the default value
for this directive by running the command-line version of PHP with the `-i` option:

```shell
php -i
```

Review the output for a line like this:

```shell
extension_dir => /usr/lib/php/modules => /usr/lib/php/modules
```

Once you've copied the extension to PHP's extension directory, you will need to enable the extension in your PHP
configuration. Your PHP installation likely supports the `/etc/php.d` configuration directory, which you can verify by
examining the output of `php -i` and looking for the presence of `--with-config-file-scan-dir` in the
"Configure Command" entry. If present, you can create a file in `/etc/php.d` that contains the directive to load the
Ice extension. For example, create the file `/etc/php.d/ice.ini` containing the following line:

```shell
extension = ice.so
```

If PHP does not support the `/etc/php.d` directory, determine the path name of PHP's configuration file as reported by
the `php -i` command:

```shell
Configuration File (php.ini) Path => /etc/php.ini
```

Open the configuration file and append the following line:

```shell
extension = ice.so
```

You can verify that PHP is loading the Ice extension by running the command
shown below:

```shell
php -m
```

Look for `ice` among the installed modules. Note that your library search path (`LD_LIBRARY_PATH` on Linux or
`DYLD_LIBRARY_PATH` on macOS) must include the directory containing the Ice shared libraries.

Read the PHP Dependencies and PHP Source Files sections below for more information about installing the Ice extension.

## PHP Dependencies

PHP needs to locate the libraries for the Ice run-time libraries and its third-party dependencies.

- Linux and macOS

```shell
libIce
libIceDiscovery
libIceLocatorDiscovery
libIceSSL
libbz2
```

In general, these libraries must reside in a directory of the user's (`LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH`
on macOS). For Web servers, the libraries may need to reside in a system directory. For example, on Linux you can add
the directory containing the Ice run-time libraries to `/etc/ld.so.conf` and run `ldconfig`.

You can verify that the Ice extension is installed properly by examining the output of the `php -m` command, or by
calling the `phpInfo()` function from a script. For example, you can create a file in the Web server's document
directory containing the following PHP script:

```php
<?php
phpInfo();
?>
```

Then start a browser window and open the URL corresponding to this script. If the Ice extension is successfully
installed, you will see an `ice` section among the configuration information.

## PHP Source Files

In addition to the binary Ice extension module and its library dependencies, you will also need to make the Ice for PHP
source files available to your scripts. These files are located in the `lib` subdirectory and consist of the Ice run
time definitions (`Ice.php`) along with PHP source files generated from the Slice files included in the Ice
distribution.

The Ice extension makes no assumptions about the location of these files, so you can install them anywhere you like.
For example, you can simply include them in the same directory as your application scripts. Alternatively, if you
prefer to install them in a common directory, you may need to modify PHP's `include_path` directive so that the PHP
interpreter is able to locate these files. For example, you could append to `php.ini`:

```ini
include_path=${include_path}";/opt/Ice-3.8.0/php/lib"
```

Another option is to modify the include path from within your script prior to including any Ice run-time file. Here is
an example that assumes Ice is installed in `/opt`:

```php
ini_set('include_path',
ini_get('include_path') . PATH_SEPARATOR . '/opt/Ice-3.8.0/php');
require 'Ice.php'; // Load the core Ice run time definitions.
```

## Running the PHP Tests

The test subdirectory contains PHP implementations of the core Ice test suite. Python is required to run the test
suite.

The test suites also require that the Ice for C++ tests be built in the `cpp` subdirectory of this source distribution.
In addition, the scripts require that the CLI version of the PHP interpreter be available in your PATH.

After a successful build, you can run the tests as follows:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

[binary distributions]: https://zeroc.com/downloads/ice
[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
