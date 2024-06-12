// swift-tools-version: 5.10

import Foundation
import PackageDescription

let iceCppSettings: [CXXSetting] = [
    .define("ICE_BUILDING_SRC"),
    .define("ICE_STATIC_LIBS"),
    .define("ICE_SWIFT"),
    .headerSearchPath("src"),
    .headerSearchPath("include"),
    .headerSearchPath("include/generated"),
]

let package = Package(
    name: "ice",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v14),
        .iOS(.v12),
    ],
    products: [
        .library(name: "Ice", targets: ["Ice"]),
        .library(name: "Glacier2", targets: ["Glacier2"]),
        .library(name: "IceGrid", targets: ["IceGrid"]),
        .library(name: "IceStorm", targets: ["IceStorm"]),
        .plugin(name: "CompileSlice", targets: ["CompileSlice"]),
    ],
    dependencies: [
        .package(url: "https://github.com/apple/swift-argument-parser.git", from: "1.4.0"),
        .package(url: "https://github.com/mxcl/PromiseKit.git", from: "8.1.2"),
        .package(name: "mcpp", path: "/Users/joe/Developer/zeroc-ice/mcpp")
    ],
    targets: [
        .target(
            name: "Ice",
            dependencies: ["IceImpl", "PromiseKit"],
            path: "swift/src/Ice",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "Glacier2",
            dependencies: ["Ice", "PromiseKit"],
            path: "swift/src/Glacier2",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceGrid",
            dependencies: ["Ice", "Glacier2", "PromiseKit"],
            path: "swift/src/IceGrid",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceStorm",
            dependencies: ["Ice", "PromiseKit"],
            path: "swift/src/IceStorm",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceImpl",
            dependencies: ["IceCpp", "IceDiscoveryCpp", "IceLocatorDiscoveryCpp"],
            path: "swift/src/IceImpl",
            cxxSettings: [
                .define("ICE_BUILDING_SRC"),
                .define("ICE_STATIC_LIBS"),
                .define("ICE_SWIFT"),
                .headerSearchPath("../../../cpp/src/"),
                .headerSearchPath("../../../cpp/include"),
                .headerSearchPath("../../../cpp/include/generated"),
            ],
            linkerSettings: [
                .linkedFramework("ExternalAccessory")
            ]
        ),
        .target(
            name: "IceCpp",
            dependencies: ["IceUtilCpp"],
            path: "cpp",
            exclude: [
                "src/Ice/build",
                "src/Ice/msbuild",
                "src/Ice/Ice.rc",
                "src/Ice/Makefile.mk",
                "src/Ice/DLLMain.cpp",
                "src/Ice/SSL/SchannelEngine.cpp",
                "src/Ice/SSL/SchannelTransceiverI.cpp",
                "src/Ice/SSL/OpenSSLTransceiverI.cpp",
                "src/Ice/SSL/OpenSSLEngine.cpp",
                "src/Ice/EventLoggerMsg.mc",
                "src/Ice/ios",
                "src/Ice/generated/BuiltinSequences.ice.d",
                "src/Ice/generated/Context.ice.d",
                "src/Ice/generated/EndpointTypes.ice.d",
                "src/Ice/generated/Identity.ice.d",
                "src/Ice/generated/Locator.ice.d",
                "src/Ice/generated/Metrics.ice.d",
                "src/Ice/generated/OperationMode.ice.d",
                "src/Ice/generated/Process.ice.d",
                "src/Ice/generated/PropertiesAdmin.ice.d",
                "src/Ice/generated/PropertyDict.ice.d",
                "src/Ice/generated/RemoteLogger.ice.d",
                "src/Ice/generated/Router.ice.d",
                "src/Ice/generated/Version.ice.d"
            ],
            sources: ["src/Ice"],
            publicHeadersPath: "include/Ice",
            cxxSettings: iceCppSettings,
            linkerSettings: [.linkedLibrary("iconv"), .linkedLibrary("bz2")]
        ),
        .target(
            name: "IceDiscoveryCpp",
            dependencies: ["IceCpp"],
            path: "cpp",
            exclude: [
                "src/IceDiscovery/build",
                "src/IceDiscovery/msbuild",
                "src/IceDiscovery/IceDiscovery.rc",
                "src/IceDiscovery/Makefile.mk",
                "src/IceDiscovery/generated/IceDiscovery.ice.d"
            ],
            sources: ["src/IceDiscovery"],
            publicHeadersPath: "src/IceDiscovery/generated",
            cxxSettings: iceCppSettings
        ),
        .target(
            name: "IceLocatorDiscoveryCpp",
            dependencies: ["IceCpp"],
            path: "cpp/",
            exclude: [
                "src/IceLocatorDiscovery/build",
                "src/IceLocatorDiscovery/msbuild",
                "src/IceLocatorDiscovery/IceLocatorDiscovery.rc",
                "src/IceLocatorDiscovery/Makefile.mk",
                "src/IceLocatorDiscovery/generated/IceLocatorDiscovery.ice.d"
            ],
            sources: ["src/IceLocatorDiscovery"],
            publicHeadersPath: "src/IceLocatorDiscovery/generated",
            cxxSettings: iceCppSettings
        ),
        .target(
            name: "IceUtilCpp",
            path: "cpp",
            exclude: [
                "src/IceUtil/build",
                "src/IceUtil/msbuild",
                "src/IceUtil/Makefile.mk"
            ],
            sources: ["src/IceUtil"],
            publicHeadersPath: "include/IceUtil",
            cxxSettings: iceCppSettings
        ),
        .target(
            name: "SliceCpp",
            dependencies: ["IceUtilCpp", "mcpp"],
            path: "cpp",
            exclude: [
                "src/Slice/build",
                "src/Slice/msbuild",
                "src/Slice/Scanner.l",
                "src/Slice/Grammar.y",
                "src/Slice/Makefile.mk",
            ],
            sources: ["src/Slice"],
            publicHeadersPath: "src/Slice",
            cxxSettings: iceCppSettings
        ),
        .executableTarget(
            name: "slice2swift",
            dependencies: ["SliceCpp", "IceUtilCpp"],
            path: "cpp",
            exclude: [
                "src/slice2swift/build",
                "src/slice2swift/msbuild",
                "src/slice2swift/Slice2Swift.rc",
            ],
            sources: ["src/slice2swift"],
            publicHeadersPath: "src/slice2swift",
            cxxSettings: iceCppSettings
        ),
        .executableTarget(
            name: "CompileSliceExecutable",
            dependencies: [
                .product(name: "ArgumentParser", package: "swift-argument-parser"),
            ],
            path: "swift/src/CompileSliceExecutable", // must be from top level to access resources
            resources: [
                .copy("slice/Ice"),
                .copy("slice/Glacier2"),
                .copy("slice/IceDiscovery"),
                .copy("slice/IceLocatorDiscovery"),
                .copy("slice/IceBox"),
                .copy("slice/IceGrid"),
                .copy("slice/IceStorm"),
            ]
        ),
        .plugin(
            name: "CompileSlice",
            capability: .buildTool(),
            dependencies: ["slice2swift", "CompileSliceExecutable"],
            path: "swift/src/CompileSlicePlugin"
        )
    ],
    swiftLanguageVersions: [SwiftVersion.v5],
    cxxLanguageStandard: .cxx17
)
