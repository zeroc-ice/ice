# Ice for Swift Build Instructions

This file describes how to build Ice for Swift from source and how to test the
resulting build.

- [Ice for Swift Build Instructions](#ice-for-swift-build-instructions)
  - [Swift Build Requirements](#swift-build-requirements)
    - [Operating Systems](#operating-systems)
    - [Swift Version](#swift-version)
  - [Building Ice for Swift](#building-ice-for-swift)
    - [Building the C++ XCFrameworks](#building-the-c-xcframeworks)
    - [Building with Make](#building-with-make)
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

Ice for Swift is built using the Swift Package Manager. The Swift Package Manager is included in the Swift toolchain,
which is included with Xcode.

### Building the C++ XCFrameworks

Building Ice for Swift requires having first built Ice for C++ from the `cpp` directory. This builds the required
C++ `XCFrameworks`:

- `Ice.xcframework`
- `IceDiscovery.xcframework`
- `IceLocatorDiscovery.xcframework`

If targeting Swift for iOS (or iOS simulator), the XCFrameworks must also be built to include the iOS architectures:

```shell
cd cpp
make PLATFORM=all
```

### Building with Make

To build Ice for Swift and its corresponding tests, run the following command from the `swift` directory:

```shell
make
```

The `PLATFORMS` argument can be used to include the [iOS test controller]:

```shell
make PLATFORM=all
```

### Building with Xcode

[Package.swift] (Ice for Swift source code), [test/Package.swift] (Ice for Swift tests), and [TestDriverApp.xcodeproj] (iOS test controller) can be opened and built using Xcode.

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

[Package.swift]: ../Package.swift
[test/Package.swift]: ./test/Package.swift
[TestDriverApp.xcodeproj]: ./test/ios/TestDriverApp.xcodeproj
[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
[iOS test controller]: ./test/ios/TestDriverApp.xcodeproj
