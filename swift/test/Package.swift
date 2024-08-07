// swift-tools-version: 5.9

import Foundation
import PackageDescription

let defaultSliceFiles = ["Test.ice"]
let defaultSources = ["Client.swift", "AllTests.swift", "Server.swift", "TestI.swift"]

struct TestConfig {
    var dependencies: [Target.Dependency] = []

    var collocated = true

    var sources = defaultSources

    var sliceFiles = defaultSliceFiles

    var resources: [Resource] = []

    var sourceFiles: [String] {
        sources + (collocated ? ["Collocated.swift"] : [])
    }

    var exclude: [String] {
        return sliceFiles
    }
}

let testDirectories: [String: TestConfig] = [
    "Ice/adapterDeactivation": TestConfig(),
    "Ice/admin": TestConfig(collocated: false),
    "Ice/ami": TestConfig(),
    "Ice/binding": TestConfig(collocated: false),
    "Ice/defaultServant": TestConfig(
        collocated: false,
        sources: ["Client.swift", "AllTests.swift"]
    ),
    "Ice/defaultValue": TestConfig(
        collocated: false,
        sources: ["Client.swift", "AllTests.swift"]
    ),
    "Ice/enums": TestConfig(collocated: false),
    "Ice/exceptions": TestConfig(),
    "Ice/facets": TestConfig(),
    "Ice/hold": TestConfig(collocated: false),
    "Ice/info": TestConfig(collocated: false),
    "Ice/inheritance": TestConfig(),
    "Ice/invoke": TestConfig(collocated: false),
    "Ice/location": TestConfig(collocated: false),
    "Ice/middleware": TestConfig(collocated: false, sources: ["Client.swift", "AllTests.swift"]),
    "Ice/objects": TestConfig(
        sliceFiles: defaultSliceFiles + ["Derived.ice", "DerivedEx.ice", "Forward.ice"]
    ),
    "Ice/operations": TestConfig(
        sources: defaultSources + [
            "BatchOneways.swift", "Oneways.swift", "Twoways.swift",
        ]
    ),
    "Ice/optional": TestConfig(
        collocated: false
    ),
    "Ice/properties": TestConfig(
        collocated: false,
        sources: ["Client.swift"],
        sliceFiles: [],
        resources: [
            .copy("config/config.1"),
            .copy("config/config.2"),
            .copy("config/config.3"),
            .copy("config/escapes.cfg"),
            .copy("config/configPath"),
        ]
    ),
    "Ice/proxy": TestConfig(),
    "Ice/retry": TestConfig(),
    "Ice/scope": TestConfig(collocated: false),
    "Ice/servantLocator": TestConfig(
        sources: defaultSources + ["ServantLocatorI.swift"]
    ),
    "Ice/services": TestConfig(collocated: false, sources: ["Client.swift"], sliceFiles: []),
    "Ice/slicing/exceptions": TestConfig(
        collocated: false,
        sliceFiles: defaultSliceFiles + ["ServerPrivate.ice"]
    ),
    "Ice/slicing/objects": TestConfig(
        collocated: false,
        sliceFiles: defaultSliceFiles + ["ClientPrivate.ice", "ServerPrivate.ice"]
    ),
    "Ice/stream": TestConfig(collocated: false, sources: ["Client.swift"]),
    "Ice/timeout": TestConfig(collocated: false),
    "Ice/udp": TestConfig(collocated: false),
    "IceSSL/configuration": TestConfig(
        collocated: false,
        resources: [
            .copy("certs")
        ]
    ),
    "Slice/escape": TestConfig(collocated: false, sources: ["Client.swift"], sliceFiles: ["Clash.ice", "Key.ice"]),
]

func testPathToTargetName(_ path: String) -> String {
    return path.replacingOccurrences(of: "/", with: "_")
}

var testDriverDependencies = [Target.Dependency]()
let testTargets = testDirectories.map { (testPath, testConfig) in

    var targets = [Target]()

    var resources = testConfig.resources

    let dependencies: [Target.Dependency] = [
        .byName(name: "TestCommon"),
        .product(name: "Ice", package: "ice"),
        .product(name: "Glacier2", package: "ice"),
        .product(name: "IceGrid", package: "ice"),
        .product(name: "IceStorm", package: "ice"),
    ]
    var plugins: [Target.PluginUsage] = []

    if testConfig.sliceFiles.count > 0 {
        plugins += [.plugin(name: "CompileSlice", package: "ice")]
        resources += [.copy("slice-plugin.json")]
    }

    let name = testPathToTargetName("\(testPath)")
    targets.append(
        Target.target(
            name: name,
            dependencies: dependencies,
            path: testPath,
            exclude: testConfig.exclude,
            sources: testConfig.sourceFiles,
            resources: resources,
            plugins: plugins
        ))
    testDriverDependencies.append(Target.Dependency(stringLiteral: name))

    return targets
}

let package = Package(
    name: "ice-test",
    defaultLocalization: "en",
    platforms: [
        .macOS(.v14),
        .iOS(.v12),
    ],
    products: [
        .library(name: "TestCommon", targets: ["TestCommon"]),
        .library(name: "TestBundle", targets: ["TestBundle"]),
    ],
    dependencies: [
        .package(name: "ice", path: "../../")
    ],
    targets: [
        .target(
            name: "TestCommon",
            dependencies: [
                .product(name: "Ice", package: "ice")
            ],
            path: "TestCommon",
            resources: [
                .process("slice-plugin.json")
            ],
            plugins: [.plugin(name: "CompileSlice", package: "ice")]
        ),
        // TestBundle is a library target that contains all the test targets
        .target(
            name: "TestBundle",
            dependencies: [.target(name: "TestCommon")] + testDriverDependencies,
            path: "TestBundle"
        ),
        .executableTarget(
            name: "TestDriver",
            dependencies: ["TestBundle"],
            path: "TestDriver"
        ),
    ],
    swiftLanguageVersions: [SwiftVersion.v5]
)

package.targets += testTargets.joined()
