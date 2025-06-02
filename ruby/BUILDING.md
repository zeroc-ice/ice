# Building Ice for Ruby from Source

## Table of Contents

* [Build roadmap](#build-roadmap)
* [Building Ice for Ruby from Source](#building-ice-for-ruby-from-source)
  * [Prerequisites](#prerequisites)
  * [Building Ice for Ruby](#building-ice-for-ruby)
  * [Running the Tests](#running-the-tests)
  * [Generating the API Reference](#generating-the-api-reference)
* [Building the Ruby Gem Package](#building-the-ruby-gem-package)

## Build roadmap

This document describes two distinct build workflows:

1. **Source Build:**
   Builds Ice for Ruby directly from the source tree.
   This build **requires a prior build of Ice for C++** and is typically used for development, testing, packaging for
   Linux distributions, and contributing to Ice for Ruby.

   ```mermaid
   flowchart LR
       c++(Ice for C++) --> ruby(Ice for Ruby)
       ruby -- doc --> api(API Reference)
       ruby -- test --> tests(Tests)
   ```

2. **Ruby Gem Package Build:**
   Builds standalone Ruby Gem packages from the source tree.
   This workflow **does not require Ice for C++** and is intended for producing distributable `.gem` packages.

## Building Ice for Ruby from Source

This build compiles Ice for Ruby directly from the source tree and requires a prior build of Ice for C++.

### Prerequisites

1. **Ruby 3.0 or higher**

2. **Ice for C++ source build**

3. **Python** (required to run the tests)

4. **[yard]** Ruby documentation tool for building the API reference

### Building Ice for Ruby

Before building Ice for Ruby, you must first build the Ice for C++ source distribution.
Refer to the [build instructions](../cpp/BUILDING.md) in the `cpp` subdirectory for details.

Once Ice for C++ is built, open a terminal and navigate to the `ruby` subdirectory.
To build Ice for Ruby, run:

```shell
make
```

### Running the Tests

To run the tests, open a terminal and change to the `ruby` subdirectory. Then run:

```shell
python allTests.py --all
```

### Generating the API Reference

To build the API reference documentation, run the following command from the `ruby` subdirectory:

```shell
yard doc 'ruby/**/*.rb'
```

## Building the Ruby Gem Package

To build the Ruby Gem package, run the following command from the `ruby` subdirectory:

```shell
rake
```

This will create the `zeroc-ice-3.8.0a.gem` file in the current directory.

[yard]: https://yardoc.org/
