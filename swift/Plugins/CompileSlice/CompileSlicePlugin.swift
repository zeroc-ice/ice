// Copyright (c) ZeroC, Inc.

import Foundation
import PackagePlugin

// The entry point for command line builds with SPM.
extension CompileSlicePlugin: BuildToolPlugin {
    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {
        guard let swiftTarget = target as? SwiftSourceModuleTarget else {
            throw PluginError.invalidTarget("\(type(of: target))")
        }

        return try createBuildCommands(
            outputDir: context.pluginWorkDirectoryURL,
            inputFiles: swiftTarget.sourceFiles,
            slice2swift: try context.tool(named: "slice2swift").url,
            iceSliceDir: CompileSlicePlugin.iceSliceDir
        )
    }
}

#if canImport(XcodeProjectPlugin)

    import XcodeProjectPlugin

    // The entry point for Xcode project builds.
    extension CompileSlicePlugin: XcodeBuildToolPlugin {
        func createBuildCommands(context: XcodePluginContext, target: XcodeTarget) throws -> [Command] {
            return try createBuildCommands(
                outputDir: context.pluginWorkDirectoryURL,
                inputFiles: target.inputFiles,
                slice2swift: try context.tool(named: "slice2swift").url,
                iceSliceDir: CompileSlicePlugin.iceSliceDir
            )
        }
    }

#endif

enum PluginError: Error {
    case invalidTarget(String)
    case missingCompiler(String)
    case missingConfigFile(String, String)
    case missingIceSliceFiles(String)

    var description: String {
        switch self {
        case .invalidTarget(let targetType):
            return "Expected a SwiftSourceModuleTarget but got '\(targetType)'."
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

/// Represents the contents of a `slice-plugin.json` file.
///
/// - `sources`: Optional list of Slice files or directories containing Slice files. Paths are relative to the
///   directory containing the `slice-plugin.json` file. Slice files declared directly in the target's source files
///   are always included and do not need to be listed here.
///
/// - `search_paths`: Optional list of directories to add as `-I` search paths when invoking `slice2swift`.
///   These paths are also relative to the config file location. Note: The Ice slice directory is automatically
///   included and does not need to be listed here.
///
/// - Example:
/// ```json
/// {
///     "sources": ["Slice"],
///     "search_paths": ["../OtherModule/Slice"]
/// }
/// ```
struct Config: Codable {
    /// Optional list of Slice files or directories containing Slice files.
    /// Paths are relative to the `slice-plugin.json` file. Slice files in the target's sources are always included.
    var sources: [String]?

    /// Optional list of directories to add as `-I` search paths when invoking `slice2swift`.
    /// Paths are relative to the `slice-plugin.json` file.
    /// Note: The Ice slice directory is automatically included.
    var search_paths: [String]?
}

/// The CompileSlicePlugin for SwiftPM compiles Ice Slice files to Swift files using the `slice2swift` compiler.
/// The `slice2swift` compilation can be configured using a `slice-plugin.json` file in the target's source files.
/// By default the plugin will compile all `.ice` files in the target's source files.
///
/// The Ice slice directory is automatically added to the search path, so you don't need to include it in
/// `search_paths`. This allows Slice files to import Ice definitions (e.g., `#include <Ice/Identity.ice>`)
/// without additional configuration.
@main
struct CompileSlicePlugin {

    /// The name of the configuration file.
    private static let configFileName = "slice-plugin.json"

    /// The Ice slice directory, derived from this plugin's source file location.
    /// Path: CompileSlicePlugin.swift -> CompileSlice -> Plugins -> swift -> (ice root) -> slice
    private static let iceSliceDir: URL? = {
        var url = URL(fileURLWithPath: #filePath)
        for _ in 0..<4 {
            url.deleteLastPathComponent()
        }
        url.append(path: "slice")
        let identityIce = url.appending(path: "Ice/Identity.ice")
        guard FileManager.default.fileExists(atPath: identityIce.path) else {
            return nil
        }
        return url
    }()

    private func createBuildCommands(
        outputDir: URL,
        inputFiles: FileList,
        slice2swift: URL,
        iceSliceDir: URL?
    ) throws -> [Command] {

        // Collect .ice input files in the target's source files.
        var sliceSources =
            inputFiles
            .filter { $0.url.pathExtension == "ice" }
            .map { $0.url }

        // Locate the config file (slice-plugin.json) in the input files.
        let configFileURL =
            inputFiles
            .first(where: { $0.url.lastPathComponent == Self.configFileName })?
            .url

        // Start with the Ice slice directory in the search path (if found).
        var searchPaths: [String] = []
        if let iceSliceDir = iceSliceDir {
            searchPaths.append("-I\(iceSliceDir.path)")
        }

        // Decode config and apply additional sources and search paths.
        if let configFileURL = configFileURL {
            let configData = try Data(contentsOf: configFileURL)
            let config = try JSONDecoder().decode(Config.self, from: configData)

            // Add additional slice files from config.sources. The files are relative to the config file location.
            let baseDirectory = configFileURL.deletingLastPathComponent()
            for source in config.sources ?? [] {
                let sourceFileOrDirectory = baseDirectory.appendingPathComponent(source)
                if sourceFileOrDirectory.pathExtension == "ice" {
                    sliceSources.append(sourceFileOrDirectory)
                } else {
                    sliceSources.append(
                        contentsOf: try FileManager.default.contentsOfDirectory(
                            at: sourceFileOrDirectory,
                            includingPropertiesForKeys: nil
                        ).filter { $0.pathExtension == "ice" })
                }
            }

            // Add additional search paths from config.search_paths.
            // These paths are relative to the config file location.
            for path in config.search_paths ?? [] {
                searchPaths.append("-I\(baseDirectory.appendingPathComponent(path).path)")
            }
        }

        // Create the build commands for each slice file.
        return sliceSources.map { sliceSource in
            let outputFile = outputDir.appendingPathComponent(sliceSource.lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")

            return .buildCommand(
                displayName: "Compile Slice \(sliceSource.lastPathComponent)",
                executable: slice2swift,
                arguments: searchPaths + ["--output-dir", outputDir.path, sliceSource.path],
                // It's important to declare both the Slice file and the config file as input files so the build system can
                // detect changes. This also avoids warnings about unused files.
                // TODO: We should also include imported Slice dependencies as inputs.
                inputFiles: [sliceSource] + (configFileURL.map { [$0] } ?? []),
                outputFiles: [URL(fileURLWithPath: outputFile.path)]
            )
        }
    }
}
