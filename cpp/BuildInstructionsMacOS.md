# Building Ice for C++ on macOS

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

 - [Berkeley DB][5] 5.3
 - [bzip][4] 1.0
 - [expat][3] 2.1
 - [mcpp][6] 2.7.2 (with patches)

Expat and bzip are included with your system. We recommend you install
Berkeley DB and mcpp with [Homebrew][7]:
```
brew install zeroc-ice/tap/berkeley-db@5.3
brew install mcpp
```

## Building Ice

In a command window, change to the `cpp` subdirectory:
```
cd cpp
```

Edit `config/Make.rules` to establish your build configuration. The comments in
the file provide more information. Pay particular attention to the variables
that define the locations of the third-party libraries.

Now you're ready to build Ice:
```
make
```

This will build the Ice core libraries, services, and tests.

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

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
pip install passlib
```

After a successful source build, you can run the tests as follows:
```
make test
```

This command is equivalent to:
```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice36/Using+the+macOS+Binary+Distribution
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+3.6.4
[3]: http://expat.sourceforge.net
[4]: http://bzip.org
[5]: http://www.oracle.com/us/products/database/berkeley-db/overview/index.htm
[6]: https://github.com/zeroc-ice/mcpp
[7]: http://brew.sh
