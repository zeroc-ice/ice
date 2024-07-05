// Copyright (c) ZeroC, Inc. All rights reserved.

import Foundation
import PackagePlugin

enum PluginError: Error {
    case invalidTarget(Target)
    case missingCompiler(String)
    case missingConfigFile(String, String)
    case missingIceSliceFiles(String)

    var description: String {
        switch self {
        case let .invalidTarget(target):
            return "Expected a SwiftSourceModuleTarget but got '\(type(of: target))'."
        case let .missingCompiler(path):
            return "Missing slice compiler: '\(path)'."
        case let .missingConfigFile(path, target):
            return
                "Missing config file '\(path)` for target `\(target)`. '. This file must be included in your sources."
        case let .missingIceSliceFiles(path):
            return "The Ice slice files are missing. Expected location: `\(path)`"
        }
    }
}

/// Represents the contents of a `slice-plugin.json` file
struct Config: Codable {
    /// List of Slice files or directories containing Slice files
    var sources: [String]
    /// List of directories to add as search paths when calling slice2swift.
    var search_paths: [String]?
}

@main
struct CompileSlicePlugin: BuildToolPlugin {

    /// The name of the configuration file
    static let configFileName = "slice-plugin.json"

    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {

        guard let sourceModuleTarget = target as? SwiftSourceModuleTarget else {
            throw PluginError.invalidTarget(target)
        }

        let sourceFiles = sourceModuleTarget.sourceFiles

        // Search for the configuration file. If this plugin was loaded, the configuration file must be present, or
        // its considered an error.
        guard
            let configFilePath = sourceFiles.first(
                where: {
                    $0.path.lastComponent == Self.configFileName
                }
            )?.path
        else {
            throw PluginError.missingConfigFile(Self.configFileName, target.name)
        }

        let data = try Data(contentsOf: URL(fileURLWithPath: configFilePath.string))
        let config = try JSONDecoder().decode(Config.self, from: data)

        let slice2swift = try context.tool(named: "slice2swift").path

        let fm = FileManager.default

        // Find the Ice Slice files for the corresponding Swift target
        let sources = try config.sources.map { source in
            let fullSourcePath = target.directory.appending(source)
            if fullSourcePath.string.hasSuffix(".ice") {
                return [fullSourcePath]
            }

            // Directory
            return try fm.contentsOfDirectory(atPath: fullSourcePath.string).filter { path in
                return path.hasSuffix(".ice")
            }.map { sliceFile in
                fullSourcePath.appending(sliceFile)
            }
        }.joined()

        let outputDir = context.pluginWorkDirectory
        let search_paths = (config.search_paths ?? []).map { "-I\(target.directory.appending($0).string)" }

        // Create a build command for each Slice file
        return sources.map { (sliceFile) -> Command in
            // Absolute path to the input Slice file
            let inputFile = sliceFile

            // Absolute path to the output Slice file
            let outputFile = Path(
                URL(fileURLWithPath: outputDir.appending(sliceFile.lastComponent).string).deletingPathExtension()
                    .appendingPathExtension("swift").relativePath)

            // Arguments for the slice2swift command
            let arguments: [String] =
                search_paths + [
                    "--output-dir",
                    outputDir.string,
                    inputFile.string,
                ]

            let displayName = slice2swift.string + " " + arguments.joined(separator: " ")

            return .buildCommand(
                displayName: displayName,
                executable: slice2swift,
                arguments: arguments,
                outputFiles: [outputFile]
            )
        }
    }
}
