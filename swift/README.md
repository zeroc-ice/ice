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

Ice for Swift builds on macOS and 

### Slice to Swift Compiler

You need the Slice to Swift compiler to build Ice for Swift and also to use
Ice for Swift. The Slice to Swift compiler (`slice2swift`) is a command-line tool
written in C++

### Swift Version

Ice for Swift requires Swift 4.2 or later.

## Building Ice for Swift

The build system requires the Slice to Swift compiler and Ice for C++ 
C++11 run-time libraries in the `cpp` subdirectory.

Open `ice.workspace` with Xcode and build the `Ice` target
