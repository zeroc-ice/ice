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
    
    /// Helper function to find slice files given a base directory and config
    private static func findSliceFiles(config: Config, baseDirectory: String) throws -> [String] {
        let fm = FileManager.default
        
        return try config.sources.map { source in
            let fullSourcePath = (baseDirectory as NSString).appendingPathComponent(source)
            if fullSourcePath.hasSuffix(".ice") {
                return [fullSourcePath]
            }
            
            // Directory - scan for .ice files
            let directoryURL = URL(fileURLWithPath: fullSourcePath)
            return try fm.contentsOfDirectory(
                at: directoryURL,
                includingPropertiesForKeys: nil,
                options: []
            ).filter { url in
                url.path.hasSuffix(".ice")
            }.map { sliceFileURL in
                sliceFileURL.path
            }
        }.joined().map { $0 }
    }
    
    /// Helper function to create build commands
    private static func createBuildCommand(
        inputFile: String,
        outputFile: String,
        outputDir: String,
        searchPaths: [String],
        slice2swiftPath: String
    ) -> Command {
        let arguments: [String] = searchPaths + [
            "--output-dir",
            outputDir,
            inputFile,
        ]
        
        let displayName = slice2swiftPath + " " + arguments.joined(separator: " ")
        
        return .buildCommand(
            displayName: displayName,
            executable: URL(fileURLWithPath: slice2swiftPath),
            arguments: arguments,
            outputFiles: [URL(fileURLWithPath: outputFile)]
        )
    }

    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {
        let fm = FileManager.default

        // Search for the configuration file
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
        let outputDir = context.pluginWorkDirectoryURL
        
        // Find slice files using helper
        let sliceFiles = try Self.findSliceFiles(config: config, baseDirectory: targetDirectoryUrl.path)
        
        // Create search paths
        let searchPaths = (config.search_paths ?? []).map {
            "-I\(targetDirectoryUrl.appendingPathComponent($0).path)"
        }

        // Create build commands using helper
        return sliceFiles.map { sliceFile in
            let outputFile = outputDir.appendingPathComponent((sliceFile as NSString).lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")
            
            return Self.createBuildCommand(
                inputFile: sliceFile,
                outputFile: outputFile.path,
                outputDir: outputDir.path,
                searchPaths: searchPaths,
                slice2swiftPath: slice2swift.path
            )
        }
    }
}

// Support for Xcode projects
#if canImport(XcodeProjectPlugin)

import XcodeProjectPlugin

extension CompileSlicePlugin: XcodeBuildToolPlugin {
    
    func createBuildCommands(context: XcodePluginContext, target: XcodeTarget) throws -> [Command] {
        
        // Find the config file among the input files
        guard let configFile = target.inputFiles.first(where: { file in
            file.path.lastComponent == Self.configFileName
        }) else {
            throw PluginError.missingConfigFile(Self.configFileName, target.displayName)
        }
        
        // Parse the configuration file
        let data = try Data(contentsOf: URL(fileURLWithPath: configFile.path.string))
        let config = try JSONDecoder().decode(Config.self, from: data)
        
        // Get the slice2swift tool
        let slice2swift = try context.tool(named: "slice2swift").path
        let outputDir = context.pluginWorkDirectory
        
        // Get the directory containing the config file as the base directory
        let targetDirectoryPath = configFile.path.removingLastComponent()
        
        // Find slice files using helper
        let sliceFiles = try Self.findSliceFiles(config: config, baseDirectory: targetDirectoryPath.string)
        
        // Create search paths
        let searchPaths = (config.search_paths ?? []).map {
            "-I\(targetDirectoryPath.appending($0).string)"
        }
        
        // Create build commands using helper, but with Xcode-specific modifications
        return sliceFiles.map { sliceFile in
            let outputFileName = ((sliceFile as NSString).lastPathComponent as NSString).deletingPathExtension + ".swift"
            let outputFile = outputDir.appending(outputFileName)
            
            let arguments: [String] = searchPaths + [
                "--output-dir",
                outputDir.string,
                sliceFile,
            ]
            
            let displayName = slice2swift.string + " " + arguments.joined(separator: " ")
            
            return .buildCommand(
                displayName: displayName,
                executable: slice2swift,
                arguments: arguments,
                inputFiles: [Path(sliceFile)],
                outputFiles: [outputFile]
            )
        }
    }
}

#endif
