// Copyright (c) ZeroC, Inc.

import Foundation
@preconcurrency import PackagePlugin

enum PluginError: Error {
    case invalidTarget(Target)
    case missingCompiler(String)
    case missingConfigFile(String, String)
    case missingIceSliceFiles(String)

    var description: String {
        switch self {
        case .invalidTarget(let target):
            return "Expected a SwiftSourceModuleTarget but got '\(type(of: target))'."
        case .missingCompiler(let path):
            return "Missing slice compiler: '\(path)'."
        case .missingConfigFile(let path, let target):
            return
                "Missing config file '\(path)` for target `\(target)`. '. This file must be included in your sources."
        case .missingIceSliceFiles(let path):
            return "The Ice slice files are missing. Expected location: `\(path)`"
        }
    }
}

/// Represents the contents of a `slice-plugin.json` file
/// - sources: List of Slice files or directories containing Slice files. Can not be empty.
/// - search_paths: List of directories to add as search paths when calling slice2swift. Can be omitted.
/// - Example:
/// ```
/// {
///     "sources": ["Slice"],
///     "search_paths": ["Slice", "../OtherModule/Slice"]
/// }
/// ```
struct Config: Codable {
    /// List of Slice files or directories containing Slice files
    var sources: [String]
    /// List of directories to add as search paths when calling slice2swift.
    var search_paths: [String]?
}

/// The CompileSlicePlugin for SwiftPM compiles Ice Slice files to Swift files using the `slice2swift` compiler.
/// The plugin expects a `slice-plugin.json` file as one of the target's sources.
/// The `slice-plugin.json` file must contain a list of Slice files or directories containing Slice files.
/// SwiftPM will compile the Slice files to Swift files before compiling Swift sources.
@main
struct CompileSlicePlugin: BuildToolPlugin {

    /// The name of the configuration file
    static let configFileName = "slice-plugin.json"

    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {

        let fm = FileManager.default

        // TODO: This should be removed once the released version of SwiftPM includes the fix.
        // Target.directoryURL is missing from the protocol
        // See https://github.com/swiftlang/swift-package-manager/issues/8001
        guard let target = target as? SwiftSourceModuleTarget else {
            fatalError("Target must be a SwiftSourceModuleTarget, but got '\(type(of: target))'.")
        }

        // Search for the configuration file. If this plugin was loaded, the configuration file must be present, or
        // it's considered an error.
        let targetDirectoryUrl = target.directoryURL
        let configFilePath = try fm.contentsOfDirectory(
            at: targetDirectoryUrl,
            includingPropertiesForKeys: nil,
            options: []
        ).first { path in
            path.lastPathComponent == Self.configFileName
        }.map { path in
            targetDirectoryUrl.appendingPathComponent(path.lastPathComponent).path
        }

        guard let configFilePath else {
            throw PluginError.missingConfigFile(Self.configFileName, target.directoryURL.path)
        }

        let data = try Data(contentsOf: URL(fileURLWithPath: configFilePath))
        let config = try JSONDecoder().decode(Config.self, from: data)

        let slice2swift = try context.tool(named: "slice2swift").url

        // Find the Ice Slice files for the corresponding Swift target
        let sources = try config.sources.map { source in
            let fullSourcePath = targetDirectoryUrl.appendingPathComponent(source)
            if fullSourcePath.path.hasSuffix(".ice") {
                return [fullSourcePath]
            }

            // Directory
            return try fm.contentsOfDirectory(
                at: fullSourcePath,
                includingPropertiesForKeys: nil,
                options: []
            ).filter { url in
                url.path.hasSuffix(".ice")
            }.map { sliceFileURL in
                fullSourcePath.appendingPathComponent(sliceFileURL.lastPathComponent)
            }
        }.joined()

        let outputDir = context.pluginWorkDirectoryURL
        let search_paths = (config.search_paths ?? []).map {
            "-I\(targetDirectoryUrl.appendingPathComponent($0).path)"
        }

        // Create a build command for each Slice file
        return sources.map { (sliceFile) -> Command in
            // Absolute path to the input Slice file
            let inputFile = sliceFile

            // Absolute path to the output Slice file
            let outputFile = outputDir.appendingPathComponent(sliceFile.lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")

            // Arguments for the slice2swift command
            let arguments: [String] =
                search_paths + [
                    "--output-dir",
                    outputDir.path,
                    inputFile.path,
                ]

            let displayName = slice2swift.path + " " + arguments.joined(separator: " ")

            return .buildCommand(
                displayName: displayName,
                executable: slice2swift,
                arguments: arguments,
                outputFiles: [outputFile]
            )
        }
    }
}
