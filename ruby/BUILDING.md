# Building Ice for Ruby from Source

## Table of Contents

- [Build roadmap](#build-roadmap)
- [Building Ice for Ruby from source](#building-ice-for-ruby-from-source)
  - [Prerequisites](#prerequisites)
  - [Building Ice for Ruby](#building-ice-for-ruby)
  - [Configuring your environment](#configuring-your-environment)
  - [Running the tests](#running-the-tests)
- [Building the Ruby gem package](#building-the-ruby-gem-package)

## Build Roadmap

This document describes two distinct build workflows:

1. **Source Build:**
   Builds Ice for Ruby directly from the source tree.
   This build **requires a prior build of Ice for C++** and is typically used for development, testing, packaging for
   Linux distributions, and contributing to Ice for Ruby.

   ```mermaid
   flowchart LR
       c++(Ice for C++) --> ruby(Ice for Ruby)
       ruby --> tests(Tests)
   ```

2. **Ruby Gem Package Build:**
   Builds standalone Ruby gem packages from the source tree.
   This workflow **does not require Ice for C++** and is intended for producing distributable `.gem` packages.

## Building Ice for Ruby from source

This build compiles Ice for Ruby directly from the source tree and requires a prior build of Ice for C++.

### Prerequisites

1. **Ruby 3.0 or later**

2. **Ice for C++ source build**

3. **Python** (required to run the tests)

### Building Ice for Ruby

Before building Ice for Ruby, you must first build the Ice for C++ source distribution.
Refer to the [build instructions](../cpp/BUILDING.md) in the `cpp` subdirectory for details.

Once Ice for C++ is built, open a terminal and navigate to the `ruby` subdirectory.
To build Ice for Ruby, run:

```shell
make
```

You can perform an automated installation with the following command:

```shell
make install
```

This process uses the `prefix` variable in `../config/Make.rules` as the installation's root directory.

### Configuring your environment

The Ruby interpreter must be able to locate the Ice extension. You need to define the `RUBYLIB` environment variable
as follows:

```shell
export RUBYLIB=/opt/Ice/ruby:$RUBYLIB
```

This example assumes that your Ice for Ruby installation is located in the `/opt/Ice` directory.

You must also modify `LD_LIBRARY_PATH` or `DYLD_LIBRARY_PATH` to include the Ice library directory.

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

To verify that Ruby can load the Ice extension successfully, open a command window and start the interpreter using
`irb`. At the prompt, enter:

```ruby
require "Ice"
```

If the interpreter responds with the value true, the Ice extension was loaded successfully. Enter `exit` to quit the
interpreter.

### Running the tests

To run the tests, open a terminal and change to the `ruby` subdirectory. Then run:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

## Building the Ruby gem package

To build the Ruby gem package, first ensure you have Rake installed:

```shell
gem install rake
```

Then run the following command from the `ruby` subdirectory:

```shell
rake build
```

This creates the gem file in the current directory. You can then install the gem:

```shell
gem install zeroc-ice-3.7.11.gem
```

The Rake build task automatically:

- Downloads and extracts the mcpp dependency
- Builds the required Slice compilers (slice2cpp, slice2rb)
- Generates the C++ and Ruby source files
- Packages everything into a gem
