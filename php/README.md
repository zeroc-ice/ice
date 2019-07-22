# Building Ice for PHP

This page describes how to build and install Ice for PHP from source.

ZeroC provides [binary distributions][1] for many platforms, including
Linux and Windows, so building Ice for PHP from source is usually
unnecessary.

* [PHP Build Requirements](#php-build-requirements)
* [Building the PHP Extension](#building-the-php-extension)
  * [Linux or macOS](#linux-or-macos)
  * [Windows](#windows)
* [Installing the PHP Extension](#installing-the-php-extension)
  * [Linux or macOS](#linux-or-macos-1)
  * [Windows](#windows-1)
* [PHP Dependencies](#php-dependencies)
* [PHP Source Files](#php-source-files)
* [Running the PHP Tests](#running-the-php-tests)
* [Web Server Permissions](#web-server-permissions)
* [SELinux Notes for PHP](#selinux-notes-for-php)

## PHP Build Requirements

Ice was extensively tested using the operating systems and compiler versions
listed on [supported platforms][2].

On Windows, the build requires a recent version of Visual Studio and the
[Ice Builder for Visual Studio][3].

## Building the PHP Extension

### Linux or macOS

The build of Ice for PHP requires that you first build Ice for C++ in the `cpp`
subdirectory.

From the top-level source directory, edit `config/Make.rules` to establish
your build configuration. The comments in the file provide more information.

Our source code only supports building Ice for PHP as a dynamic PHP extension;
the product of the build is a shared library that you must configure PHP to
load.

Change to the `php` source subdirectory:
```
cd php
```

Ensure that `php` and `php-config` for the version of PHP you wish to
build against are first in your PATH.

Run `make` to build the extension.

### Windows

Open a Visual Studio command prompt. For example, with Visual Studio 2015, you
can open one of:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while
the second Command Promt produces `x64` binaries by default.

In the Command Prompt, change to the `php` subdirectory:
```
cd php
```

Now you're ready to build Ice for PHP:
```
msbuild msbuild\ice.proj
```

This builds the extension with `Release` binaries for the default platform. The
extension will be placed in `lib\x64\Release\php_ice.dll` for the `x64` platform
and `lib\Win32\Release\php_ice.dll` for the `Win32` platform.

The default configuration builds the extension against the thread-safe PHP run
time. You can build with the non-thread-safe run time using the `NTS-Release` or
`NTS-Debug` configuration:
```
msbuild msbuild\ice.proj /p:Configuration=NTS-Release
```

The extension will be placed in `lib\x64\Release\php_ice_nts.dll` directory for
the `x64` platform and `lib\Win32\Release\php_ice_nts.dll` for the `Win32`
platform.

The extension is built by default with namespaces enabled. You can build the PHP
extension with namespaces disabled by setting the MSBuild property
`PhpUseNamespaces` to `no`:
```
msbuild msbuild\ice.proj /p:PhpUseNamespaces=no
```

The PHP extension is build by default using PHP 7.1, the extension can be build with
PHP 7.1, 7.2 or 7.3 by setting MSBuildProperty `BuildWithPhpVersion` to the desired
version for example you can use the following command to build the extension with PHP
7.2:

```
msbuild msbuild\ice.proj /p:BuildWithPhpVersion=7.2
```

The PHP extension depends on Ice for C++ components from the `cpp` subdirectory,
and those are built if required. It is also possible to build the PHP extension
using the Ice for C++ NuGet packages by setting the`ICE_BIN_DIST` msbuild
property to `cpp`:
```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=cpp
```

## Installing the PHP Extension

### Linux or macOS

To install the Ice extension, you must move the extension's shared library into
PHP's extension directory. This directory is determined by the PHP configuration
directive `extension_dir`. You can determine the default value for this
directive by running the command-line version of PHP with the `-i` option:
```
php -i
```

Review the output for a line like this:
```
extension_dir => /usr/lib/php/modules => /usr/lib/php/modules
```

Once you've copied the extension to PHP's extension directory, you will need to
enable the extension in your PHP configuration. Your PHP installation likely
supports the `/etc/php.d` configuration directory, which you can verify by
examining the output of `php -i` and looking for the presence of
`--with-config-file-scan-dir` in the "Configure Command" entry. If present,
you can create a file in `/etc/php.d` that contains the directive to load the
Ice extension. For example, create the file `/etc/php.d/ice.ini` containing
the following line:
```
extension = ice.so
```

If PHP does not support the `/etc/php.d` directory, determine the path name of
PHP's configuration file as reported by the `php -i` command:
```
Configuration File (php.ini) Path => /etc/php.ini
```

Open the configuration file and append the following line:
```
extension = ice.so
```

You can verify that PHP is loading the Ice extension by running the command
shown below:
```
php -m
````

Look for `ice` among the installed modules. Note that your library search path
(`LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS) must include the
directory containing the Ice shared libraries.

Read the PHP Dependencies and PHP Source Files sections below for more
information about installing the Ice extension.

### Windows

To install the Ice extension, you must move the extension's shared library into
PHP's extension directory. This directory is determined by the PHP configuration
directive `extension_dir`. You can determine the default value for this
directive by running the command-line version of PHP with the `-i` option:
```
php -i
```

Review the output for a line like this:
```
extension_dir => C:\Program Files\iis express\PHP\v7.1\ext\ => C:\Program Files\iis express\PHP\v7.1\ext\
```

Once you've copied the extension to the appropriate directory, you will need
to enable the extension in your PHP configuration. First you must discover the
location of PHP's configuration file (`php.ini`), which is also displayed by
the `-i` option. Look for the following line:
```
Loaded Configuration File => C:\Program Files\iis express\PHP\v7.1\php.ini
```

Open `php.ini` and append the following directive:
```
extension=php_ice_nts.dll
```

Read the PHP Dependencies and PHP Source Files sections below for more
information about installing the Ice extension.

## PHP Dependencies

PHP needs to locate the libraries for the Ice run-time libraries and its
third-party dependencies.

* Linux and macOS
```
libIce
libIceDiscovery
libIceLocatorDiscovery
libIceSSL
libbz2
```

* Windows
```
bzip2.dll
ice37.dll
icediscovery37.dll
icelocatordiscovery37.dll
icessl37.dll
```

In general, these libraries must reside in a directory of the user's PATH.
For Web servers, the libraries may need to reside in a system directory. For
example, on Linux you can add the directory containing the Ice run-time
libraries to `/etc/ld.so.conf` and run `ldconfig`.

For IIS configured to run PHP as FastCGI, the simplest solution is to copy the
libraries next to the `php-cgi.exe` in `C:\Program Files\iis express\PHP\v7.1`.

You can verify that the Ice extension is installed properly by examining the
output of the `php -m` command, or by calling the `phpInfo()` function from a
script. For example, you can create a file in the Web server's document
directory containing the following PHP script:
```
<?php
phpInfo();
?>
```

Then start a browser window and open the URL corresponding to this script. If
the Ice extension is successfully installed, you will see an `ice` section
among the configuration information.

## PHP Source Files

In addition to the binary Ice extension module and its library dependencies,
you will also need to make the Ice for PHP source files available to your
scripts. These files are located in the `lib` subdirectory and consist of the
Ice run time definitions (`Ice.php`) along with PHP source
files generated from the Slice files included in the Ice distribution.

The Ice extension makes no assumptions about the location of these files, so you
can install them anywhere you like. For example, you can simply include them in
the same directory as your application scripts. Alternatively, if you prefer to
install them in a common directory, you may need to modify PHP's `include_path`
directive so that the PHP interpreter is able to locate these files. For example,
you could append to `php.ini`:
```
include_path=${include_path}";C\ice\php\lib"
```

Another option is to modify the include path from within your script prior to
including any Ice run-time file. Here is an example that assumes Ice is installed
in
`/opt`:
```
// PHP
ini_set('include_path',
ini_get('include_path') . PATH_SEPARATOR . '/opt/Ice/php');
require 'Ice.php'; // Load the core Ice run time definitions.
```

## Running the PHP Tests

The test subdirectory contains PHP implementations of the core Ice test suite.
Python is required to run the test suite.

The test suites also require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution. In addition, the scripts require
that the CLI version of the PHP interpreter be available in your PATH.

After a successful build, you can run the tests as follows:
```
python allTests.py
```

If you have built the extension using the Ice for C++ NuGet packages, you must
also set the `ICE_BIN_DIST` environment variable to `cpp` for testing:
```
set ICE_BIN_DIST=cpp
python allTests.py
```

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
```
# setenforce 0
```

to disable SELinux until the next reboot of your computer.

If you want to run httpd with Ice for PHP and SELinux enabled, there are two
steps you need to take. First, allow httpd to open network connections:
```
# setsebool httpd_can_network_connect=1
```

Add the `-P` option to make this setting persistent across reboots.

Second, make sure any `.ice` file used by your PHP scripts can be read by httpd.
The enclosing directory also needs to be accessible. For example:
```
# chcon -R -t httpd_sys_content_t /opt/MyApp/slice
```

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.3
[3]: https://github.com/zeroc-ice/ice-builder-visualstudio
