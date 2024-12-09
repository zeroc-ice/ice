# Ice for Swift Build Instructions

This file describes how to build Ice for Swift from source and how to test the
resulting build.

- [Ice for Swift Build Instructions](#ice-for-swift-build-instructions)
  - [Swift Build Requirements](#swift-build-requirements)
    - [Operating Systems](#operating-systems)
    - [Swift Version](#swift-version)
  - [Building Ice for Swift](#building-ice-for-swift)
    - [Building with Xcode](#building-with-xcode)
  - [Running the Swift Test Suite](#running-the-swift-test-suite)
    - [macOS](#macos)
    - [iOS](#ios)

## Swift Build Requirements

### Operating Systems

Ice for Swift builds and runs on macOS and is supported on the platforms listed
on the [supported platforms] page.

### Swift Version

Ice for Swift requires Swift 5 or later.

## Building Ice for Swift

Building Ice for Swift requires having built Ice for C++ first from the `cpp` directory. This builds required the
`XCFramework`s.

To build Ice for Swift and its corresponding tests, run the following command form the `swift` directory:

```shell
make
```

The `PLATFORMS` argument can be used to include the [iOS test controller]:

```shell
make PLATFORM=all
```

### Building with Xcode

`Package.swift` (Ice for Swift source code), `test/Package.swift` (Ice for Swift tests), and `test/ios/TestDriverApp.xcodeproj` (iOS test controller) can be opened and built using Xcode.

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

[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
[iOS test controller]: ./test/ios/TestDriverApp.xcodeproj
