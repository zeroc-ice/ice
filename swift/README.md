# Building Ice for Swift

This file describes how to build Ice for Swift from source and how to test the
resulting build.

* [Build Requirements](#build-requirements)
  * [Operating Systems](#operating-systems)
  * [Slice to Swift Compiler](#slice-to-swift-compiler)
  * [Swift Version](#swift-version)
* [Building Ice for Swift](#building-ice-for-swift)

## Swift Build Requirements

### Operating Systems

Ice for Swift builds and runs properly on macOS and is fully supported on
the platforms listed on the [supported platforms][2] page.

### Slice to Swift Compiler

You need the Slice to Swift compiler to build Ice for Swift and also to use
Ice for Swift. The Slice to Swift compiler (`slice2swift`) is a command-line tool
written in C++. You can build the Slice to Swift compiler from source, or
alternatively you can install an Ice [binary distribution][1] that includes
this compiler.

### Swift Version

Ice for Swift requires Swift 5 or later.

## Building Ice for Swift

The build system requires the Slice to Swift and the Slice to C++
compilers. If you have not install a binary distribution that provides
this command-line tools, refer to the [C++ build instructions](../cpp/README.md)
to build these compilers using the Ice for C++ sources.

Open `ice.xcproject` with Xcode and build the `Ice macOS` or `Ice iOS` target
to build Ice for Swift macOS framework or Ice for Swift iOS framework respectively.

The tests programs for macOS and iOS can be build using the `TestDriver macOS` and
`TestDriver iOS` respectively

## Running the Swift Tests on

Python is required to run the test suite.

### macOS

```
python allTests.py --config Debug
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

### iOS

Start the `TestDriver iOS` app on your iOS device, from Xcode.

```
python allTests.py --config Debug --platform iphoneos
```

## Install

### Carthage

In your `Cartfile` add a reference to Ice package
```
github "zeroc-ice/ice" ~> swift
```

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.2
