// swift-tools-version: 6.1

import Foundation
import PackageDescription

let package = Package(
    name: "Ice",
    platforms: [
        .macOS(.v10_14),
        .iOS(.v12),
    ],
    products: [
        .library(name: "Ice", targets: ["Ice"]),
        .library(name: "Glacier2", targets: ["Glacier2"]),
        .library(name: "IceStorm", targets: ["IceStorm"]),
        .library(name: "IceGrid", targets: ["IceGrid"])
    ],
    dependencies: [
        .package(url: "https://github.com/mxcl/PromiseKit.git", from: "6.8.10")
    ],
    targets: [
        // Swift targets
        .target(
            name: "Ice",
            dependencies: ["IceImpl", "PromiseKit"],
            path: "Sources/Ice"
        ),
        .target(
            name: "Glacier2",
            dependencies: ["Ice"],
            path: "Sources/Glacier2"
        ),
        .target(
            name: "IceStorm",
            dependencies: ["Ice"],
            path: "Sources/IceStorm"
        ),
        .target(
            name: "IceGrid",
            dependencies: ["Ice", "Glacier2"],
            path: "Sources/IceGrid"
        ),

        // Objective-C++ bridge layer
        .target(
            name: "IceImpl",
            dependencies: ["IceCpp", "IceSSLCpp", "IceDiscoveryCpp", "IceLocatorDiscoveryCpp"],
            path: "Sources/IceImpl",
            linkerSettings: [
                .linkedLibrary("iconv"),
                .linkedLibrary("bz2"),
                .linkedFramework("ExternalAccessory"),
                .linkedFramework("Security")
            ]
        ),

        // Pre-built XCFrameworks (path placeholders replaced by CI with url/checksum)
        .binaryTarget(
            name: "IceCpp",
            path: "Ice.xcframework"
        ),
        .binaryTarget(
            name: "IceSSLCpp",
            path: "IceSSL.xcframework"
        ),
        .binaryTarget(
            name: "IceDiscoveryCpp",
            path: "IceDiscovery.xcframework"
        ),
        .binaryTarget(
            name: "IceLocatorDiscoveryCpp",
            path: "IceLocatorDiscovery.xcframework"
        ),
    ],
    cxxLanguageStandard: .cxx14
)
