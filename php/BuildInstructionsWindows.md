# Building Ice for PHP on Windows

This page describes how to build and install Ice for PHP from source code on
Windows. If you prefer, you can also download [binary distributions][1] for the
supported platforms.

## PHP Build Requirements

### Operating Systems and Compilers

Ice for PHP was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][8], you must install
version 4.2.0 or greater to build Ice.

### Ice Development Kit

You will need the Ice development kit for C++, which you can install as a binary
distribution or compile from source yourself.

## Building the PHP Extension

The most common motivation for compiling the Ice extension yourself is to create
an extension that is compatible with your existing environment, such as when
your Web server or PHP interpreter is built with a different compiler.

To build the Ice extension, first download the PHP binary archive or Windows
installer and install the distribution.

You will also need to download and extract the PHP sources (Ice for PHP
requires the PHP header files). Change to the PHP source directory and run the
following commands:

    > buildconf
    > configure

You do not need to actually build PHP from source, but these two steps are
necessary to generate header files that are required by the Ice extension.

You must built Ice for C++ from the `cpp` subdirectory, if you have not done so
review cpp\BuildInstructionsWindows.md first.

Change to the Ice for PHP source directory:

    > cd php

Building the extension:

There is a number of MSbuild properties that controls the building of the
extension, review `ice.pros` to stablish your build configuration.

    > MSbuild msbuild\ice.proj

This will build the extension in `Release` configuration and using the command
prompt default platform, for `x64` platform the extension will be placed in
`lib\x64\Release\php_ice.dll` and for `Win32` platform the extension will be
placed in `lib\Win32\Release\php_ice.dll`.


If you want to build a debug version of the extension you can to so by setting
the MSBuild `Configuration` property to `Debug`:

    > MSbuild msbuild\ice.proj /p:Configuration=Debug

the extension will be placed in `lib\x64\Debug\php_ice.dll` directory and for
`Win32` platform the extension will be placed in `lib\Win32\Release\php_ice.dll`.

## Installing the PHP Extension

To install the Ice extension, you must move the extension's shared library into
PHP's extension directory. This directory is determined by the PHP configuration
directive `extension_dir`. You can determine the default value for this
directive by running the command-line version of PHP with the `-i` option:

    > php -i

Review the output for a line like this:

    extension_dir => C:\php => C:\php

The directive may also be set to a relative path, such as:

    extension_dir => ./ => ./

In the case of a relative path, the value is relative to the current working
directory of the process. As a result, the working directory when running the
command-line version of PHP will likely differ from the working directory when
PHP is running as a Web server module. Using a typical installation of Apache
as an example, the working directory is Apache's installation directory,
therefore the extension must be copied to

    \Program Files\Apache Software Foundation\Apache2.2

Once you've copied the extension to the appropriate directory, you will need
to enable the extension in your PHP configuration. First you must discover the
location of PHP's configuration file (`php.ini`), which is also displayed by
the `-i` option. Look for the following line:

    Loaded Configuration File => C:\Program Files\PHP\php.ini

If you used the Windows installer for PHP, your Web server's configuration may
have already been modified to load PHP. You can also review your Web server's
settings to discover the location of `php.ini`. For example, PHP's Windows
installer modifies Apache's configuration to add the following directives:

    PHPIniDir "C:/Program Files/PHP/"
    LoadModule php5_module "C:/Program Files/PHP/php5apache2_2.dll"

The `PHPIniDir` directive specifies the directory containing the `php.ini` file.

Open `php.ini` and append this directive:

    extension = php_ice.dll

Read the PHP Dependencies and PHP Source Files sections below for more
information about installing the Ice extension.

## PHP Dependencies

PHP will need to be able to locate the libraries for the Ice run-time libraries
and its third-party dependencies. On Windows, these DLLs are required:

    ice37.dll
    iceutil37.dll
    slice37.dll
    bzip2.dll

In general, these libraries must reside in a directory of the user's PATH. For
Web servers, the libraries may need to reside in a system directory. For
example, on Windows you can copy the DLLs to the `C:\WINDOWS\system32`
directory, or to the Apache installation directory.

You can verify that the Ice extension is installed properly by examining the
output of the `php -m` command, or by calling the `phpInfo()` function from a
script. For example, you can create a file in the Web server's document
directory containing the following PHP script:

    <?php
    phpInfo();
    ?>

Then start a browser window and open the URL corresponding to this script. If
the Ice extension is successfully installed, you will see an `ice` section
among the configuration information.

Note that if you want to use IceSSL from the Ice extension, then PHP will also
need access to the shared libraries for IceSSL and OpenSSL.

## PHP Source Files

In addition to the binary Ice extension module and its library dependencies,
you will also need to make the Ice for PHP source files available to your
scripts. These files are located in the `lib` subdirectory and consist of the
Ice run time definitions (`Ice.php` or `Ice_ns.php`) along with PHP source
files generated from the Slice files included in the Ice distribution.

The Ice extension makes no assumptions about the location of these files, so
you can install them anywhere you like. For example, you can simply include
them in the same directory as your application scripts. Alternatively, if you
prefer to install them in a common directory, you may need to modify PHP's
`include_path` directive so that the PHP interpreter is able to locate these
files. Another option is to modify the include path from within your script
prior to including any Ice run-time file. Here is an example that assumes
Ice is installed in `C:\IcePHP`:

    // PHP
    ini_set('include_path',
    ini_get('include_path') . PATH_SEPARATOR . 'C:/IcePHP');
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

For example, on Windows the Apache server typically runs as a service in the
"Local System" account. You will need to modify the access rights of the
aforementioned files to grant access to this account. In a command window,
you can use the `cacls` utility to establish the appropriate access rights.
Assuming that you have copied the Ice extension and dependent DLLs to Apache's
installation directory, you can modify the access rights as shown below:

    cd \Program Files\Apache Software Foundation\Apache2.2
    cacls php_ice.dll /G SYSTEM:F Administrators:F
    cacls bzip2.dll /G SYSTEM:F Administrators:F
    ...

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
