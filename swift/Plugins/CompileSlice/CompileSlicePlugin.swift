// Copyright (c) ZeroC, Inc.

import Foundation
import PackagePlugin

// The entry point for command line builds with SPM
extension CompileSlicePlugin: BuildToolPlugin {
    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {
        guard let swiftTarget = target as? SwiftSourceModuleTarget else {
            throw PluginError.invalidTarget("\(type(of: target))")
        }

        return try createBuildCommands(
            outputDirUrl: context.pluginWorkDirectoryURL,
            inputFiles: swiftTarget.sourceFiles,
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
                outputDirUrl: context.pluginWorkDirectoryURL,
                inputFiles: target.inputFiles,
                slice2swiftUrl: try context.tool(named: "slice2swift").url
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
            return "Expected a SwiftSourceModuleTarget but got '\(targetType))'."
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
///   These paths are also relative to the config file location.
///
/// - Example:
/// ```json
/// {
///     "sources": ["Slice"],
///     "search_paths": ["Slice", "../OtherModule/Slice"]
/// }
/// ```
struct Config: Codable {
    /// Optional list of Slice files or directories containing Slice files.
    /// Paths are relative to the `slice-plugin.json` file. Slice files in the target's sources are always included.
    var sources: [String]?

    /// Optional list of directories to add as `-I` search paths when invoking `slice2swift`.
    /// Paths are relative to the `slice-plugin.json` file.
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
        for source in config.sources ?? [] {
            let url = URL(fileURLWithPath: baseDirectory).appendingPathComponent(source)
            if url.pathExtension == "ice" {
                sliceFiles.append(url)
            } else {
                sliceFiles.append(
                    contentsOf: try fm.contentsOfDirectory(
                        at: url,
                        includingPropertiesForKeys: nil
                    ).filter { $0.pathExtension == "ice" })
            }
        }
        return sliceFiles
    }

    private func createBuildCommands(
        outputDirUrl: URL,
        inputFiles: FileList,
        slice2swiftUrl: URL
    ) throws -> [Command] {

        // Collect .ice input files
        var sliceSources =
            inputFiles
            .filter { $0.url.pathExtension == "ice" }
            .map { $0.url }

        // Locate the config file URL
        let configFileURL =
            inputFiles
            .first(where: { $0.url.lastPathComponent == Self.configFileName })?
            .url

        var config: Config? = nil
        var searchPaths: [String] = []

        // Decode config
        if let configFileURL = configFileURL {
            let configData = try Data(contentsOf: configFileURL)
            config = try JSONDecoder().decode(Config.self, from: configData)

            // Files are relative to the config file's
            let baseDirectory = configFileURL.deletingLastPathComponent()

            // Add additional slice files from config
            sliceSources.append(
                contentsOf:
                    try Self.findSliceFiles(config: config!, baseDirectory: baseDirectory.path)
            )

            // Prepare -I search paths from config
            searchPaths = (config!.search_paths ?? []).map {
                "-I\(baseDirectory.appendingPathComponent($0).path)"
            }
        }

        // Create build commands for each slice file
        return sliceSources.map { sliceSource in
            let outputFile = outputDirUrl.appendingPathComponent(sliceSource.lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")

            return .buildCommand(
                displayName: "Compile Slice \(sliceSource.lastPathComponent)",
                executable: URL(fileURLWithPath: slice2swiftUrl.path),
                arguments: searchPaths + ["--output-dir", outputDirUrl.path, sliceSource.path],
                inputFiles: [sliceSource] + (configFileURL.map { [$0] } ?? []),
                outputFiles: [URL(fileURLWithPath: outputFile.path)],
            )
        }
    }
}
