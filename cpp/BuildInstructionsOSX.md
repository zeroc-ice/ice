# Building Ice for C++ on OS X

This page describes the Ice source distribution, including information about compiler requirements, third-party dependencies, and instructions for building and testing the distribution. If you prefer, you can install a [Homebrew](https://doc.zeroc.com/display/Ice36/Using+the+OS+X+Binary+Distribution) package instead.

## C++ Build Requirements for OS X

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions listed for our [supported platforms](https://zeroc.com/platforms_3_6_0.html).

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [expat](http://expat.sourceforge.net/) 2.0
 - [OpenSSL](http://openssl.org) 0.9.8 or later
 - [bzip](http://bzip.org) 1.0
 - [Berkeley DB](http://www.oracle.com/us/products/database/berkeley-db/overview/index.htm) 5.3
 - [mcpp](https://github.com/zeroc-ice/mcpp) 2.7.2 (with patches)

Expat, OpenSSL and bzip are included with your system. For Berkeley DB and mcpp, you have a couple of options:

- Using [Homebrew](http://brew.sh), install Berkeley DB and mcpp with these commands:

        $ brew tap zeroc-inc/ice```
        $ brew install berkeley-db53 [--without-java]```
        $ brew install mcpp```
    
   The ```berkeley-db53``` package is a pre-compiled bottle that includes Java support by default; you can exclude Java support using the ```--without-java``` option.
- Download the Berkeley DB and mcpp source distributions and build them yourself.

## Compiling and Testing Ice for C++ on OS X

In a command window, change to the ```cpp``` subdirectory:

    $ cd cpp

Edit ```config/Make.rules``` to establish your build configuration. The comments in the file provide more information. Pay particular attention to the variables that define the locations of the third-party libraries.

Now you're ready to build Ice:

    $ make

This will build the Ice core libraries, services, and tests.

Python is required to run the test suite. After a successful build, you can run the tests as follows:

    $ make test

This command is equivalent to:

    $ python allTests.py

If everything worked out, you should see lots of ```ok``` messages. In case of a failure, the tests abort with ```failed```.

## Installing a C++ Source Build on OS X

Simply run ```make install```. This will install Ice in the directory specified by the ```prefix``` variable in ```config/Make.rules```.

After installation, make sure that the ```prefix/bin``` directory is in your ```PATH```.

If you choose to not embed a ```runpath``` into executables at build time (see your build settings in ```config/Make.rules```) or did not create a symbolic link from the ```runpath``` directory to the installation directory, you also need to add the library directory to your ```DYLD_LIBRARY_PATH```.

When compiling Ice programs, you must pass the location of the ```prefix/include``` directory to the compiler with the ```-I``` option, and the location of the library directory with the ```-L``` option.
