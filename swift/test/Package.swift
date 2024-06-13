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
    "Ice/servantLocator",
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
    ],
    "IceSSL/configuration": []
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
    let dependencies: [Target.Dependency] = [
        .byName(name: "TestCommon"),
        .product(name: "Ice", package: "ice"),
        .product(name: "Glacier2", package: "ice"),
        .product(name: "IceGrid", package: "ice"),
        .product(name: "IceStorm", package: "ice"),
    ]
    var plugins: [Target.PluginUsage] = []

    if let extraResources = extraTestResources[testPath] {
        print("Adding extra resources for \(testPath)")
        print(extraResources)
        resources += extraResources
    }

    let sliceFiles = filterSliceFiles(testPath)

    if sliceFiles.count > 0 {
        plugins += [.plugin(name: "CompileSlice", package: "ice")]
        exclude += sliceFiles
        resources += [.copy("slice-plugin.json")]
    }

    let isAmdFile: (String) -> Bool = { path in
        return path.hasSuffix("AMDI.swift") || path.hasSuffix("AMD.swift")
    }

    let existingAmdFiles = findAllFiles(testPath).filter(isAmdFile)

    let hasAmd = existingAmdFiles.count > 0

    if hasAmd {
        exclude += existingAmdFiles + ["amd"]
    }

    let name = testPathToTargetName("\(testPath)")
    targets.append(
        Target.target(
            name: name,
            dependencies: dependencies,
            path: testPath,
            exclude: exclude,
            resources: resources,
            plugins: plugins
        ))
    testDriverDependencies.append(Target.Dependency(stringLiteral: name))

    if hasAmd {
        let amdName = name + "AMD"
        let amdExclude = findAllFiles(testPath).filter {
            !isAmdFile($0) && $0 != "amd/slice-plugin.json"
        }

        targets.append(
            Target.target(
                name: amdName,
                dependencies: dependencies,
                path: testPath,
                exclude: amdExclude,
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
            path: "TestCommon"
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
