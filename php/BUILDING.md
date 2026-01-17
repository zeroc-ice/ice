# Building Ice for PHP from Source

This file describes how to build and install Ice for PHP from source.
ZeroC provides [binary distributions] for many platforms, including Linux, so building Ice for PHP from source is
usually unnecessary.

## Table of Contents

- [Build roadmap](#build-roadmap)
- [Building Ice for PHP from source](#building-ice-for-php-from-source)
  - [Prerequisites](#prerequisites)
  - [Building Ice for PHP](#building-ice-for-php)
  - [Installing Ice for PHP](#installing-ice-for-php)
  - [Configuring the Ice for PHP extension](#configuring-the-ice-for-php-extension)
  - [Running the tests](#running-the-tests)

## Build roadmap

```mermaid
flowchart LR
    c++(Ice for C++) --> php(Ice for PHP)
    php --> tests(Tests)
```

## Building Ice for PHP from source

This build compiles Ice for PHP directly from the source tree and requires a prior build of Ice for C++.

### Prerequisites

1. **PHP 7.1 or later**

2. **Ice for C++ source build**

3. **Python** (required to run the tests)

### Building Ice for PHP

Before building Ice for PHP, you must first build the Ice for C++ source distribution.
Refer to the [build instructions](../cpp/BUILDING.md) in the `cpp` subdirectory for details.

Once Ice for C++ is built, open a terminal and navigate to the `php` subdirectory.

Ensure that `php` and `php-config` for the version of PHP you wish to build against are first in your PATH.

To build Ice for PHP, run:

```shell
make
```

### Installing Ice for PHP

To install the Ice extension, you must move the extension's shared library into PHP's extension directory. This
directory is determined by the PHP configuration directive `extension_dir`. You can determine the default value for
this directive by running:

```shell
php -i
```

Review the output for a line like this:

```shell
extension_dir => /usr/lib/php/modules => /usr/lib/php/modules
```

You can also perform an automated installation with the following command:

```shell
make install
```

This process uses the `prefix` variable in `../config/Make.rules` as the installation's root directory.

### Configuring the Ice for PHP extension

Once you've copied the extension to PHP's extension directory, you will need to enable the extension in your PHP
configuration. Your PHP installation likely supports the `/etc/php.d` configuration directory, which you can verify by
examining the output of `php -i` and looking for the presence of `--with-config-file-scan-dir` in the "Configure
Command" entry. If present, you can create a file in `/etc/php.d` that contains the directive to load the Ice
extension.

For example, create the file `/etc/php.d/ice.ini` containing the following line:

```ini
extension = ice.so
```

If PHP does not support the `/etc/php.d` directory, you need to edit your `php.ini` file directly. Run `php -i` and
look for "Loaded Configuration File" to find its location, then append the `extension = ice.so` line to that file.

You can verify that PHP is loading the Ice extension by running:

```shell
php -m
```

Look for `ice` among the installed modules.

For PHP to successfully load the `ice.so` extension, the Ice shared libraries (`libIce`, `libIceDiscovery`, and
`libIceLocatorDiscovery`) must also be accessible via your system's dynamic linker path. If they are not, you need to
add the Ice library directory to your library path:

**On Linux (RHEL, SLES, Amazon):**

```shell
export LD_LIBRARY_PATH=/opt/Ice/lib64:$LD_LIBRARY_PATH
```

**On Linux (Debian, Ubuntu):**

```shell
export LD_LIBRARY_PATH=/opt/Ice/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
```

**On macOS:**

```shell
export DYLD_LIBRARY_PATH=/opt/Ice/lib:$DYLD_LIBRARY_PATH
```

In addition to the binary Ice extension module and its library dependencies, you will also need to make the Ice for PHP
source files available to your scripts. These files are located in the `lib` subdirectory and consist of the Ice run
time definitions (`Ice.php`) along with PHP source files generated from the Slice files included in the Ice
distribution.

The Ice extension makes no assumptions about the location of these files, so you can install them anywhere you like.
For example, you can simply include them in the same directory as your application scripts. Alternatively, if you
prefer to install them in a common directory, you may need to modify PHP's `include_path` directive so that the PHP
interpreter is able to locate these files. For example, you could append to `php.ini`:

```ini
include_path=${include_path}";C\ice\php\lib"
```

Another option is to modify the include path from within your script prior to including any Ice run-time file. Here is
an example that assumes Ice is installed in `/opt/Ice`:

```php
ini_set('include_path', ini_get('include_path') . PATH_SEPARATOR . '/opt/Ice/php');
require 'Ice.php'; // Load the core Ice run time definitions.
```

### Running the tests

The test subdirectory contains PHP implementations of the core Ice test suite.

The test suites also require that the Ice for C++ tests be built in the `cpp` subdirectory of this source distribution.
In addition, the scripts require that the CLI version of the PHP interpreter be available in your PATH.

To run the tests, open a terminal and change to the `php` subdirectory. Then run:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

[binary distributions]: https://zeroc.com/downloads/ice
