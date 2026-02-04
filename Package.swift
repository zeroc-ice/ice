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
            url: "https://download.zeroc.com/ice/3.8/Ice-3.8.0.xcframework.zip",
            checksum: "9ae4a1f6acdd9bb2b8313248fc608d00eb92e12a4c54c11620b3b632f75d2ff1"
        ),
        .binaryTarget(
            name: "IceDiscoveryCpp",
            url: "https://download.zeroc.com/ice/3.8/IceDiscovery-3.8.0.xcframework.zip",
            checksum: "b8ae3ce32d97e835442f4d9c8349dc351228e8f505bb1fe14787a6249c53bd2e"

        ),
        .binaryTarget(
            name: "IceLocatorDiscoveryCpp",
            url: "https://download.zeroc.com/ice/3.8/IceLocatorDiscovery-3.8.0.xcframework.zip",
            checksum: "fafbab3290984aee378087b041d256f846e3b8461e56fc1e667546a4dc44ecde"
        ),
        .binaryTarget(
            name: "slice2swift",
            path: "cpp/bin/slice2swift.artifactbundle.zip"
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
