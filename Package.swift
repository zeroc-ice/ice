// swift-tools-version: 6.1

import Foundation
import PackageDescription

let iceUtilSources: [String] = [
    "src/Ice/ConsoleUtil.cpp",
    "src/Ice/CtrlCHandler.cpp",
    "src/Ice/Demangle.cpp",
    "src/Ice/Exception.cpp",
    "src/Ice/FileUtil.cpp",
    "src/Ice/LocalException.cpp",
    "src/Ice/Options.cpp",
    "src/Ice/OutputUtil.cpp",
    "src/Ice/Random.cpp",
    "src/Ice/StringConverter.cpp",
    "src/Ice/StringUtil.cpp",
    "src/Ice/UUID.cpp",
]

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
        .package(url: "https://github.com/zeroc-ice/mcpp.git", branch: "master"),
        .package(url: "https://github.com/apple/swift-docc-plugin", from: "1.4.3"),
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
            path: "cpp/lib/XCFrameworks/Ice.xcframework"
        ),
        .binaryTarget(
            name: "IceDiscoveryCpp",
            path: "cpp/lib/XCFrameworks/IceDiscovery.xcframework"

        ),
        .binaryTarget(
            name: "IceLocatorDiscoveryCpp",
            path: "cpp/lib/XCFrameworks/IceLocatorDiscovery.xcframework"
        ),
        .executableTarget(
            name: "slice2swift",
            dependencies: ["mcpp"],
            path: "cpp",
            exclude: [
                "test",
                "src/slice2swift/build",
                "src/slice2swift/msbuild",
                "src/slice2swift/Slice2Swift.rc",
                "src/Slice/build",
                "src/Slice/msbuild",
                "src/Slice/Scanner.l",
                "src/Slice/Grammar.y",
                "src/Slice/Makefile.mk",
            ],
            sources: ["src/slice2swift", "src/Slice"] + iceUtilSources,
            publicHeadersPath: "src/slice2swift",
            cxxSettings: [
                .headerSearchPath("src"),
                .headerSearchPath("include"),
            ],
            linkerSettings: [
                // See https://github.com/zeroc-ice/ice/issues/3900
                .unsafeFlags(["-Xlinker", "-max_default_common_align", "-Xlinker", "0x4000"])
            ]
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
