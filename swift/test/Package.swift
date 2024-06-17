// swift-tools-version: 5.10

import Foundation
import PackageDescription

let defaultSliceFiles = ["Test.ice"]
let defaultSources = ["Client.swift", "AllTests.swift", "Server.swift", "TestI.swift"]

let defaultAMDSliceFiles = ["TestAMD.ice"]
let defaultAMDSourceFiles = ["ServerAMD.swift", "TestAMDI.swift"]

struct TestConfig {
    var dependencies: [Target.Dependency] = []

    var collocated = true

    var sources = defaultSources

    var sliceFiles = defaultSliceFiles

    var resources: [Resource] = []

    var amd: Bool = false
    var amdSliceFiles = defaultAMDSliceFiles
    var amdSourcesFiles = defaultAMDSourceFiles

    var sourceFiles: [String] {
        sources + (collocated ? ["Collocated.swift"] : [])
    }

    var exclude: [String] {
        if !amd {
            return []
        }
        return amdSourcesFiles + ["amd"]
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
    "Ice/exceptions": TestConfig(amd: true),
    "Ice/facets": TestConfig(collocated: true),
    "Ice/hold": TestConfig(collocated: false),
    "Ice/info": TestConfig(collocated: false),
    "Ice/inheritance": TestConfig(),
     "Ice/invoke": TestConfig(collocated: false),
     "Ice/location": TestConfig(collocated: false),
    "Ice/objects": TestConfig(),
    "Ice/operations": TestConfig(
        sources: defaultSources + ["BatchOneways.swift", "BatchOnewaysAMI.swift", "Oneways.swift", "OnewaysAMI.swift", "Twoways.swift", "TwowaysAMI.swift"],
        amd: true
    ),
    "Ice/optional": TestConfig(
        collocated: false,
        amd: true
    ),
    "Ice/properties": TestConfig(
        collocated: false,
        sources: ["Client.swift"],
        sliceFiles: [],
        resources: [
            Resource.copy("config/config.1"),
            Resource.copy("config/config.2"),
            Resource.copy("config/config.3"),
            Resource.copy("config/escapes.cfg"),
            Resource.copy("config/configPath"),
        ]
    ),
    "Ice/proxy": TestConfig(amd: true),
    "Ice/retry": TestConfig(),
    "Ice/scope": TestConfig(collocated: false),
    "Ice/servantLocator": TestConfig(
        sources: defaultSources + ["ServantLocatorI.swift"],
        amd: true,
        amdSourcesFiles: defaultAMDSourceFiles + ["ServantLocatorAMDI.swift"]
    ),
    "Ice/services": TestConfig(collocated: false, sources: ["Client.swift"], sliceFiles: []),
    "Ice/slicing/exceptions": TestConfig(
        collocated: false,
        sliceFiles: defaultSliceFiles + ["ServerPrivate.ice"],
        amd:true,
        amdSliceFiles: defaultAMDSliceFiles + ["ServerPrivateAMD.ice"]
    ),
    "Ice/slicing/objects": TestConfig(
        collocated: false,
        sliceFiles: defaultSliceFiles + ["ClientPrivate.ice"],
        amd:true,
        amdSliceFiles: defaultAMDSliceFiles + ["ClientPrivateAMD.ice"]
    ),
    "Ice/stream": TestConfig(collocated: false, sources: ["Client.swift"]),
    "Ice/timeout": TestConfig(collocated: false),
    "Ice/udp": TestConfig(collocated: false),
    // "IceSSL/configuration",
    "Slice/escape": TestConfig(collocated:false, sources:["Client.swift"], sliceFiles: ["Clash.ice", "Key.ice"])
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
        resources += testConfig.resources + [.copy("slice-plugin.json")]
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

     if testConfig.amd {
         let amdName = name + "AMD"
         targets.append(
             Target.target(
                 name: amdName,
                 dependencies: dependencies,
                 path: testPath,
                 exclude: testConfig.sourceFiles,
                 resources: [.copy("amd/slice-plugin.json")],
                 plugins: plugins
             ))
         testDriverDependencies.append(Target.Dependency(stringLiteral: amdName))
     }

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
            plugins: [.plugin(name: "CompileSlice", package: "ice")]
        ),
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
