// swift-tools-version: 5.9

import Foundation
import PackageDescription

let defaultSliceFiles = ["Test.ice"]
let defaultSources = ["Client.swift", "AllTests.swift", "Server.swift", "TestI.swift"]

struct TestConfig {
    var dependencies: [Target.Dependency] = []

    var collocated = false

    var sources = defaultSources

    var sliceFiles = defaultSliceFiles

    var resources: [Resource] = []
}

let testDirectories: [String: TestConfig] = [
    "Ice/adapterDeactivation": TestConfig(),
    "Ice/admin": TestConfig(),
    "Ice/ami": TestConfig(collocated: true),
    "Ice/binding": TestConfig(),
    "Ice/defaultServant": TestConfig(
        sources: ["Client.swift", "AllTests.swift"]
    ),
    "Ice/defaultValue": TestConfig(
        sources: ["Client.swift", "AllTests.swift"]
    ),
    "Ice/enums": TestConfig(),
    "Ice/exceptions": TestConfig(collocated: true),
    "Ice/facets": TestConfig(collocated: true),
    "Ice/hold": TestConfig(),
    "Ice/info": TestConfig(),
    "Ice/inheritance": TestConfig(collocated: true),
    "Ice/invoke": TestConfig(),
    "Ice/location": TestConfig(),
    "Ice/middleware": TestConfig(sources: ["Client.swift", "AllTests.swift"]),
    "Ice/objects": TestConfig(
        collocated: true,
        sliceFiles: defaultSliceFiles + ["Derived.ice", "DerivedEx.ice", "Forward.ice"]
    ),
    "Ice/operations": TestConfig(
        collocated: true,
        sources: defaultSources + [
            "BatchOneways.swift", "Oneways.swift", "Twoways.swift",
        ]
    ),
    "Ice/optional": TestConfig(),
    "Ice/properties": TestConfig(
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
    "Ice/proxy": TestConfig(collocated: true),
    "Ice/retry": TestConfig(collocated: true),
    "Ice/scope": TestConfig(),
    "Ice/servantLocator": TestConfig(
        collocated: true,
        sources: defaultSources + ["ServantLocatorI.swift"]
    ),
    "Ice/services": TestConfig(sources: ["Client.swift"], sliceFiles: []),
    "Ice/slicing/exceptions": TestConfig(
        sliceFiles: defaultSliceFiles + ["ServerPrivate.ice"]
    ),
    "Ice/slicing/objects": TestConfig(
        sliceFiles: defaultSliceFiles + ["ClientPrivate.ice", "ServerPrivate.ice"]
    ),
    "Ice/stream": TestConfig(sources: ["Client.swift"]),
    "Ice/timeout": TestConfig(),
    "Ice/udp": TestConfig(),
    "Slice/escape": TestConfig(sources: ["Client.swift"], sliceFiles: ["Clash.ice", "Key.ice"]),
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

    let sources = testConfig.sources + (testConfig.collocated ? ["Collocated.swift"] : [])

    targets.append(
        Target.target(
            name: name,
            dependencies: dependencies,
            path: testPath,
            sources: sources,
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
