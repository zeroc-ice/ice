// swift-tools-version: 5.9

import Foundation
import PackageDescription

let iceCppSettings: [CXXSetting] = [
    .headerSearchPath("src"),
    .headerSearchPath("include"),
    .headerSearchPath("include/generated"),
]

let iceUtilSources: [String] = [
    "src/Ice/ConsoleUtil.cpp",
    "src/Ice/CtrlCHandler.cpp",
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
        .package(url: "https://github.com/zeroc-ice/mcpp.git", branch: "master"),
        .package(url: "https://github.com/apple/swift-docc-plugin", from: "1.1.0"),
    ],
    targets: [
        .target(
            name: "Ice",
            dependencies: ["IceImpl"],
            path: "swift/src/Ice",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "Glacier2",
            dependencies: ["Ice"],
            path: "swift/src/Glacier2",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceGrid",
            dependencies: ["Ice", "Glacier2"],
            path: "swift/src/IceGrid",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceStorm",
            dependencies: ["Ice"],
            path: "swift/src/IceStorm",
            resources: [.process("slice-plugin.json")],
            plugins: [.plugin(name: "CompileSlice")]
        ),
        .target(
            name: "IceImpl",
            dependencies: ["IceCpp", "IceDiscoveryCpp", "IceLocatorDiscoveryCpp"],
            path: "swift/src/IceImpl",
            cxxSettings: [
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
                "test",
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
                "src/Ice/generated/Version.ice.d",
                "src/IceIAP/Makefile.mk",
            ] + iceUtilSources,
            sources: ["src/Ice", "src/IceIAP"],
            publicHeadersPath: "include/Ice",
            cxxSettings: iceCppSettings,
            linkerSettings: [.linkedLibrary("iconv"), .linkedLibrary("bz2")]
        ),
        .target(
            name: "IceDiscoveryCpp",
            dependencies: ["IceCpp"],
            path: "cpp",
            exclude: [
                "test",
                "src/IceDiscovery/build",
                "src/IceDiscovery/msbuild",
                "src/IceDiscovery/IceDiscovery.rc",
                "src/IceDiscovery/Makefile.mk",
                "src/IceDiscovery/generated/IceDiscovery.ice.d",
            ],
            sources: ["src/IceDiscovery"],
            publicHeadersPath: "src/IceDiscovery/generated",
            cxxSettings: iceCppSettings
        ),
        .target(
            name: "IceLocatorDiscoveryCpp",
            dependencies: ["IceCpp"],
            path: "cpp",
            exclude: [
                "test",
                "src/IceLocatorDiscovery/build",
                "src/IceLocatorDiscovery/msbuild",
                "src/IceLocatorDiscovery/IceLocatorDiscovery.rc",
                "src/IceLocatorDiscovery/Makefile.mk",
                "src/IceLocatorDiscovery/generated/IceLocatorDiscovery.ice.d",
            ],
            sources: ["src/IceLocatorDiscovery"],
            publicHeadersPath: "src/IceLocatorDiscovery/generated",
            cxxSettings: iceCppSettings
        ),

        .target(
            name: "IceUtilCpp",
            path: "cpp",
            exclude: [
                "test",
                "src/IceUtil/build",
                "src/IceUtil/msbuild",
                "src/IceUtil/Makefile.mk",
            ],
            sources: iceUtilSources,
            publicHeadersPath: "src/IceUtil",  // dummy path as we can't re-use include/Ice
            cxxSettings: iceCppSettings
        ),
        .target(
            name: "SliceCpp",
            dependencies: ["IceUtilCpp", "mcpp"],
            path: "cpp",
            exclude: [
                "test",
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
                "test",
                "src/slice2swift/build",
                "src/slice2swift/msbuild",
                "src/slice2swift/Slice2Swift.rc",
            ],
            sources: ["src/slice2swift"],
            publicHeadersPath: "src/slice2swift",
            cxxSettings: iceCppSettings
        ),
        .plugin(
            name: "CompileSlice",
            capability: .buildTool(),
            dependencies: ["slice2swift"],
            path: "swift/src/CompileSlicePlugin"
        ),
    ],
    swiftLanguageVersions: [SwiftVersion.v5],
    cxxLanguageStandard: .cxx20
)
