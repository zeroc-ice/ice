// Copyright (c) ZeroC, Inc.

import Foundation
@preconcurrency import PackagePlugin

// The entry point for command line builds with SPM
extension CompileSlicePlugin: BuildToolPlugin {
    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {
        return try createBuildCommands(
            outputDirUrl: context.pluginWorkDirectoryURL,
            targetDirectoryUrl: target.directoryURL,
            slice2swiftUrl: try context.tool(named: "slice2swift").url
        )
    }
}

#if canImport(XcodeProjectPlugin)

import XcodeProjectPlugin

// The entry point for Xcode project builds.
extension CompileSlicePlugin: XcodeBuildToolPlugin {
    func createBuildCommands(context: XcodePluginContext, target: XcodeTarget) throws -> [Command] {
        return try createBuildCommands(
            pluginWorkDirectoryUrl: context.pluginWorkDirectoryURL,
            targetDirectoryUrl: target.directoryURL,
            slice2swiftUrl: try context.tool(named: "slice2swift").url
        )
    }
}

#endif

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
struct CompileSlicePlugin {

    /// The name of the configuration file
    private static let configFileName = "slice-plugin.json"
    
    /// Helper function to find slice files given a base directory and config
    private static func findSliceFiles(config: Config, baseDirectory: String) throws -> [URL] {
        let fm = FileManager.default
        
        var sliceFiles: [URL] = []
        for source in config.sources {
            let url = URL(fileURLWithPath: baseDirectory).appendingPathComponent(source)
            if url.pathExtension == "ice" {
                sliceFiles.append(url)
            } else {
                sliceFiles.append(contentsOf: try fm.contentsOfDirectory(
                    at: url,
                    includingPropertiesForKeys: nil).filter { $0.pathExtension == "ice" })
            }
        }
        return sliceFiles
    }

    private func createBuildCommands(
        outputDirUrl: URL,
        targetDirectoryUrl: URL,
        slice2swiftUrl: URL
    ) throws -> [Command] {
        let fm = FileManager.default
        let configFilePath = try fm.contentsOfDirectory(
            at: targetDirectoryUrl,
            includingPropertiesForKeys: nil,
            options: []
        ).first { path in
            path.lastPathComponent == Self.configFileName
        }.map { path in
            path
        }

        // If the configuration file is missing we report an error
        guard let configFilePath else {
            throw PluginError.missingConfigFile(Self.configFileName, targetDirectoryUrl.path)
        }

        let data = try Data(contentsOf: configFilePath)
        let config = try JSONDecoder().decode(Config.self, from: data)
        
        // Find slice files using helper
        let sliceFiles = try Self.findSliceFiles(config: config, baseDirectory: targetDirectoryUrl.path)
        
        // Create search paths
        let searchPaths = (config.search_paths ?? []).map {
            "-I\(targetDirectoryUrl.appendingPathComponent($0).path)"
        }

        // Create build commands using helper
        return sliceFiles.map { sliceFileURL in
            let outputFile = outputDirUrl.appendingPathComponent(sliceFileURL.lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")

            return .buildCommand(
                displayName: "Compile Slice \(sliceFileURL.lastPathComponent)",
                executable: URL(fileURLWithPath: slice2swiftUrl.path),
                arguments: searchPaths + ["--output-dir", outputDirUrl.path, sliceFileURL.path],
                outputFiles: [URL(fileURLWithPath: outputFile.path)],
            )
        }
    }
}
