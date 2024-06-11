// swift-tools-version: 5.10

import Foundation
import PackageDescription

// Finds all files in the given path recursively
func findAllFiles(_ path: String) -> [String] {
    var files = [String]()
    let url = URL(fileURLWithPath: path)
    if let enumerator = FileManager.default.enumerator(
        at: url, includingPropertiesForKeys: [.isRegularFileKey],
        options: [.skipsHiddenFiles, .skipsPackageDescendants])
    {
        for case let fileURL as URL in enumerator {
            do {
                let fileAttributes = try fileURL.resourceValues(forKeys: [.isRegularFileKey])
                if fileAttributes.isRegularFile! {
                    if var relativePath = fileURL.absoluteString.removingPercentEncoding {
                        relativePath.trimPrefix(url.absoluteString.removingPercentEncoding!)
                        files.append(relativePath)
                    }
                }
            } catch { print(error, fileURL) }
        }
    }
    return files
}

let testDirectories = [
    "Ice/adapterDeactivation",
    "Ice/admin",
    "Ice/ami",
    "Ice/binding",
    "Ice/defaultServant",
    "Ice/defaultValue",
    "Ice/enums",
    "Ice/exceptions",
    "Ice/facets",
    "Ice/hold",
    "Ice/info",
    "Ice/inheritance",
    "Ice/invoke",
    "Ice/location",
    "Ice/objects",
    "Ice/operations",
    "Ice/optional",
    "Ice/properties",
    "Ice/proxy",
    "Ice/retry",
    "Ice/scope",
    //    "Ice/servantLocator",
    "Ice/services",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Ice/stream",
    "Ice/timeout",
    "Ice/udp",
    // "IceSSL/configuration",
    "Slice/escape",
]

let extraTestResources = [
    "Ice/properties": [
        Resource.copy("config/config.1"),
        Resource.copy("config/config.2"),
        Resource.copy("config/config.3"),
        Resource.copy("config/escapes.cfg"),
        Resource.copy("config/configPath"),
    ]
    // "IceSSL/configuration": findAllFiles("swift/test/IceSSL/certs").filter{ $0.hasSuffix(".pem") || $0.hasSuffix(".p12")  }.map {
    //     Resource.copy($0)
    // },
]

func testPathToTargetName(_ path: String) -> String {
    return path.replacingOccurrences(of: "/", with: "_")
}

func filterSliceFiles(_ path: String) -> [String] {
    return findAllFiles(path).filter { p in p.hasSuffix(".ice") }
}

var testDriverDependencies = [Target.Dependency]()
let testTargets = testDirectories.map { testPath in
    var targets = [Target]()

    var resources = [Resource]()
    var exclude = [String]()
    let dependencies: [Target.Dependency] = ["TestCommon", "Ice", "Glacier2", "IceStorm", "IceGrid"]
    var plugins: [Target.PluginUsage] = []

    if let extraResources = extraTestResources[testPath] {
        resources += extraResources
    }

    let fullPath = "swift/test/\(testPath)"

    let sliceFiles = filterSliceFiles(fullPath)

    if sliceFiles.count > 0 {
        plugins += [.plugin(name: "CompileSlice")]
        exclude += sliceFiles
        resources += [.copy("slice-plugin.json")]
    }

    let amdServerFiles = ["TestAMDI.swift", "ServerAMD.swift"]

    let existingAmdFiles = findAllFiles(fullPath).filter { amdServerFiles.contains($0) }

    let hasAmd = existingAmdFiles.count > 0

    if hasAmd {
        exclude += existingAmdFiles + ["amd"]
    }

    if testPath == "IceSSL/configuration" {
        print(resources)
    }

    let name = testPathToTargetName("\(testPath)")
    targets.append(
        Target.target(
            name: name,
            dependencies: dependencies,
            path: fullPath,
            exclude: exclude,
            resources: resources,
            plugins: plugins
        ))
    testDriverDependencies.append(Target.Dependency(stringLiteral: name))

    if hasAmd {
        let amdName = name + "AMD"
        let amdExclude = findAllFiles(fullPath).filter {
            !amdServerFiles.contains($0) && $0 != "amd/slice-plugin.json"
        }

        targets.append(
            Target.target(
                name: amdName,
                dependencies: dependencies,
                path: fullPath,
                exclude: amdExclude,
                resources: [.copy("amd/slice-plugin.json")],
                plugins: plugins
            ))
        testDriverDependencies.append(Target.Dependency(stringLiteral: amdName))
    }

    return targets
}

let iceCppSettings: [CXXSetting] = [
    .define("ICE_BUILDING_SRC"),
    .define("ICE_STATIC_LIBS"),
    .define("ICE_SWIFT"),
    .headerSearchPath("src"),
    .headerSearchPath("include"),
    .headerSearchPath("include/generated"),
]

var cppExclude = [
    "src/Ice/msbuild",
    "src/Ice/build",
    "src/Ice/DLLMain.cpp",
    "src/Ice/SSL/SchannelEngine.cpp",
    "src/Ice/SSL/SchannelTransceiverI.cpp",
    "src/Ice/SSL/OpenSSLTransceiverI.cpp",
    "src/Ice/SSL/OpenSSLEngine.cpp",
]

findAllFiles("cpp").filter { !$0.hasSuffix(".cpp") && !$0.hasSuffix(".h") }.forEach {
    cppExclude.append($0)
}

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
        .plugin(name: "CompileSlice", targets: ["CompileSlice"]),
    ],
    dependencies: [
        .package(url: "https://github.com/mxcl/PromiseKit.git", from: "8.1.2"),
        .package(name: "mcpp", path: "/Users/joe/Developer/zeroc-ice/mcpp"),
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
            dependencies: ["SliceCpp"],
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
        .plugin(
            name: "CompileSlice",
            capability: .buildTool(),
            dependencies: ["slice2swift"],
            path: "swift/src/CompileSlicePlugin"
        ),
        .executableTarget(
            name: "TestDriver",
            dependencies: ["TestCommon"] + testDriverDependencies,
            path: "swift/test/TestDriver"
        ),
        .target(
            name: "TestCommon",
            dependencies: ["Ice"],
            path: "swift/test/TestCommon"
        ),
    ],
    swiftLanguageVersions: [SwiftVersion.v5],
    cxxLanguageStandard: .cxx17
)

package.targets += testTargets.joined()
