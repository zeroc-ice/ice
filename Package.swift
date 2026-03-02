// swift-tools-version: 6.1

import Foundation
import PackageDescription

let package = Package(
    name: "ice",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v15),
        .iOS(.v18),
    ],
    products: [
        .library(name: "Ice", targets: ["Ice"]),
        .library(name: "Glacier2", targets: ["Glacier2"]),
        .library(name: "IceGrid", targets: ["IceGrid"]),
        .library(name: "IceBox", targets: ["IceBox"]),
        .library(name: "IceStorm", targets: ["IceStorm"]),
        .plugin(name: "CompileSlice", targets: ["CompileSlice"]),
    ],
    dependencies: [
        .package(url: "https://github.com/apple/swift-docc-plugin", from: "1.4.3")
    ],
    targets: [
        .target(
            name: "Ice",
            dependencies: ["IceImpl"],
            path: "swift/src/Ice",
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "Glacier2",
            dependencies: ["Ice"],
            path: "swift/src/Glacier2",
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceGrid",
            dependencies: ["Ice", "Glacier2"],
            path: "swift/src/IceGrid",
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceBox",
            dependencies: ["Ice"],
            path: "swift/src/IceBox",
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceStorm",
            dependencies: ["Ice"],
            path: "swift/src/IceStorm",
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceImpl",
            dependencies: [
                "IceCpp",
                "IceDiscoveryCpp",
                "IceLocatorDiscoveryCpp",
            ],
            path: "swift/src/IceImpl",
            linkerSettings: [
                .linkedLibrary("bz2"),
                .linkedFramework("ExternalAccessory"),
            ]
        ),
        .binaryTarget(
            name: "IceCpp",
            url: "https://download.zeroc.com/ice/3.8/Ice-3.8.1.xcframework.zip",
            checksum: "22d6ddc85a895321a8fb12b7cac06c12504b56849cef198517324342be719ad6"
        ),
        .binaryTarget(
            name: "IceDiscoveryCpp",
            url: "https://download.zeroc.com/ice/3.8/IceDiscovery-3.8.1.xcframework.zip",
            checksum: "943a3293a08fdf4d94288519c0d9d4f8815dcfd9bd83d18743b3c8361655f5cc"

        ),
        .binaryTarget(
            name: "IceLocatorDiscoveryCpp",
            url: "https://download.zeroc.com/ice/3.8/IceLocatorDiscovery-3.8.1.xcframework.zip",
            checksum: "371a9a8cfccb84666eba311a46e37c81d3a071dc036cfc4ba78df8e9f9e37f4a"
        ),
        .binaryTarget(
            name: "slice2swift",
            url: "https://download.zeroc.com/ice/3.8/slice2swift-3.8.1.artifactbundle.zip",
            checksum: "9efbeafe341c8c99c92c1fc20d4cea69dac54627dc7a9aeaa7877ad15e512ec6"
        ),
        .plugin(
            name: "CompileSlice",
            capability: .buildTool(),
            dependencies: ["slice2swift"],
            path: "swift/Plugins/CompileSlice"
        ),
    ],
    swiftLanguageModes: [.v6],
    cxxLanguageStandard: .cxx20
)
