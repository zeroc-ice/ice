// swift-tools-version: 5.9

import Foundation
import PackageDescription

let package = Package(
    name: "ice",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v14),
        .iOS(.v17),
    ],
    products: [
        .library(name: "Ice", targets: ["Ice"]),
        .library(name: "Glacier2", targets: ["Glacier2"]),
        .library(name: "IceGrid", targets: ["IceGrid"]),
        .library(name: "IceStorm", targets: ["IceStorm"]),
        .plugin(name: "CompileSlice", targets: ["CompileSlice"]),
    ],
    dependencies: [
        .package(url: "https://github.com/zeroc-ice/mcpp.git", branch: "master"),
    ],
    targets: [
        .target(
            name: "Ice",
            dependencies: ["IceImpl"],
            exclude: ["slice-plugin.json"],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "Glacier2",
            dependencies: ["Ice"],
            exclude: ["slice-plugin.json"],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceGrid",
            dependencies: ["Ice", "Glacier2"],
            exclude: ["slice-plugin.json"],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceStorm",
            dependencies: ["Ice"],
            exclude: ["slice-plugin.json"],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceImpl",
            dependencies: [
                "IceCpp",
                "IceDiscoveryCpp",
                "IceLocatorDiscoveryCpp",
            ],
            linkerSettings: [
                .linkedLibrary("bz2"),
                .linkedFramework("ExternalAccessory")
            ]
        ),
        .binaryTarget(
            name: "IceCpp",
            url: "${Ice_XCFRAMEWORK_URL}",
            checksum: "${Ice_XCFRAMEWORK_CHECKSUM}"
        ),
        .binaryTarget(
            name: "IceDiscoveryCpp",
            url: "${IceDiscovery_XCFRAMEWORK_URL}",
            checksum: "${IceDiscovery_XCFRAMEWORK_CHECKSUM}"

        ),
        .binaryTarget(
            name: "IceLocatorDiscoveryCpp",
            url: "${IceLocatorDiscovery_XCFRAMEWORK_URL}",
            checksum: "${IceLocatorDiscovery_XCFRAMEWORK_CHECKSUM}"
        ),
        .executableTarget(
            name: "slice2swift",
            dependencies: ["mcpp"],
            path: "cpp"
        ),
        .plugin(
            name: "CompileSlice",
            capability: .buildTool(),
            dependencies: ["slice2swift"]
        ),
    ],
    swiftLanguageVersions: [SwiftVersion.v5],
    cxxLanguageStandard: .cxx20
)
