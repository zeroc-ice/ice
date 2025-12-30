# Ice for Swift Build Instructions

This file describes how to build Ice for Swift from source and how to test the
resulting build.

- [Swift Build Requirements](#swift-build-requirements)
  - [Operating Systems](#operating-systems)
  - [Slice to Swift Compiler](#slice-to-swift-compiler)
  - [Swift Version](#swift-version)
- [Building Ice for Swift](#building-ice-for-swift)
- [Running the Swift Test Suite](#running-the-swift-test-suite)
  - [macOS](#macos)
  - [iOS](#ios)

## Swift Build Requirements

### Operating Systems

Ice for Swift builds and runs on macOS and is supported on the platforms listed
on the [supported platforms] page.

### Slice to Swift Compiler

You need the Slice to Swift compiler to build Ice for Swift and also to use Ice
for Swift. The Slice to Swift compiler (`slice2swift`) is a command-line tool
written in C++. You can build the Slice to Swift compiler from source, or
alternatively you can install an Ice [binary distribution] that includes this
compiler.

### Swift Version

Ice for Swift requires Swift 5 or later.

## Building Ice for Swift

Open `ice.xcodeproj` with Xcode and build the `Ice macOS` or `Ice iOS`
targets. The PromiseKit dependency will be automatically fetched via Swift
Package Manager.

The test programs for macOS and iOS can be built using `TestDriver macOS` and
`TestDriver iOS` respectively.

Building the `TestDriver iOS` application to deploy to an iOS device requires
signing the application with a developer certificate. You need to update the Xcode projects
to use your Apple development certificates by setting `DEVELOPMENT_TEAM` environment
variable to the Id of your development team and regenerate the projects with `rake`.

```shell
export DEVELOPMENT_TEAM=U4TBVKNQ7F
rake
```

## Running the Swift Test Suite

Python is required to run the test suite.

### macOS

After a successful build, you can run the tests as follows:

```shell
python3 allTests.py --config Debug
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

### iOS

Start the `TestDriver iOS` application on your iOS device or simulator, from
Xcode.

Then on your mac, run:

```shell
python3 allTests.py --config Debug --platform iphoneos
```

or

```shell
python3 allTests.py --config Debug --platform iphonesimulator
```

depending on your target.

[binary distribution]: https://zeroc.com/downloads/ice
[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
