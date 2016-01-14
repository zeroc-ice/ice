# Building Ice for C++ on OS X

This file describes the Ice source distribution, including information about
compiler requirements, third-party dependencies, and instructions for building
and testing the distribution. If you prefer, you can install a [Homebrew][1]
package instead.

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed for our [supported platforms][2].

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip][3] 1.0
 - [Expat][4] 2.1
 - [LMDB][5] 0.9.16 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 (with patches)

Expat and bzip are included with your system. 

You can install LMDB and mcpp using [Homebrew][7]:

    brew install lmdb mcpp

## Building Ice

In a command window, change to the `cpp` subdirectory:

    cd cpp

Edit `config/Make.rules` to establish your build configuration. The comments in
the file provide more information. Pay particular attention to the variables
that define the locations of the third-party libraries.

Now you're ready to build Ice:

    make

This will build the Ice core libraries, services, and tests.

### C++11 mapping

The C++ source tree supports two different language mappings (C++98 and C++11), 
the default build uses the C++98 map. The C++11 mapping is a new mapping that
uses the new language features.

To build the new C++11 mapping, set the environment variable `CPP11_MAPPING` to
yes, as shown below:

    export CPP11_MAPPING=yes

## Installing a C++ Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `prefix` variable in `config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your
`PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `DYLD_LIBRARY_PATH`.

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

If building a C++11 program, you must define `ICE_CPP11_MAPPING` macro during
compilation with the `-D` option as `g++ -DICE_CPP11_MAPING ` and add the `/c++11`
suffix to the library directory when linking (such as `-L<prefix>/lib/c++11`).

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    pip install passlib

After a successful source build, you can run the tests as follows:

    make test

This command is equivalent to:

    python allTests.py

For C++11 mapping it also include the`--c++11` argument:

    $ python allTests.py --c++11

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice37/Using+the+OS+X+Binary+Distribution
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: http://bzip.org
[4]: http://expat.sourceforge.net
[5]: http://symas.com/mdb/
[6]: https://github.com/zeroc-ice/mcpp
[7]: http://brew.sh
