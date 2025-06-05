# Build Ice for Swift

This file describes how to build Ice for Swift from source and how to test the resulting build.

- [Building](#building)
  - [Prerequisites](#prerequisites)
  - [Building with Make](#building-with-make)
  - [Building with Xcode](#building-with-xcode)
- [Running the Swift Test Suite](#running-the-swift-test-suite)
  - [macOS](#macos)
  - [iOS](#ios)

## Building

### Prerequisites

1. Swift and Xcode\
   You need Swift 5 or later with a recent version of Xcode for macOS.

2. Ice C++ XCFrameworks\
   Building Ice for Swift requires having first built Ice for C++ from the `cpp` directory. The Ice for C++ build
   creates the following C++ `XCFrameworks`:
    - `Ice.xcframework`
    - `IceDiscovery.xcframework`
    - `IceLocatorDiscovery.xcframework`

   If you want to create binaries for iOS or the iOS simulator, build the C++ XCFrameworks with `PLATFORMS=all`:

   ```shell
   make -C ../cpp -j12 PLATFORMS=all
   ```

### Building with make

Run the following command from the `swift` directory:

```shell
make
```

Add `PLATFORMS='macosx iphonesimulator'` to include the [iOS test controller] in your build:

```shell
make PLATFORMS='macosx iphonesimulator'
```

### Building with Xcode

[Package.swift] (Ice for Swift source code), [test/Package.swift] (Ice for Swift tests), and [TestDriverApp.xcodeproj]
(iOS test controller) can be opened and built using Xcode.

## Running the Swift Test Suite

Python is required to run the test suite.

### macOS

After a successful build, you can run the tests as follows:

```shell
python allTests.py --all --config Debug
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

### iOS

Start the `TestDriver iOS` application on your iOS device or simulator, from Xcode.

Then on your mac, run:

```shell
python allTests.py --all --config Debug --platform iphoneos
```

or

```shell
python allTests.py --all --config Debug --platform iphonesimulator
```

depending on your target.

[Package.swift]: ../Package.swift
[test/Package.swift]: ./test/Package.swift
[TestDriverApp.xcodeproj]: ./test/ios/TestDriverApp.xcodeproj
[iOS test controller]: ./test/ios/TestDriverApp.xcodeproj
