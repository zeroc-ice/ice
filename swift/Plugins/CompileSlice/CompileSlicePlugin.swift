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
            )
        }
    }

#endif

enum PluginError: Error, CustomStringConvertible, LocalizedError {
    case invalidTarget(String)
    case missingIceSliceFiles(String)
    case dependencyScanFailed(String)

    var description: String {
        switch self {
        case .invalidTarget(let targetType):
            return "Expected a SwiftSourceModuleTarget but got '\(targetType)'."
        case .missingIceSliceFiles(let path):
            return "The Ice Slice files are missing. Expected location: '\(path)'."
        case .dependencyScanFailed(let reason):
            return "Could not determine the Slice include dependencies: \(reason)."
        }
    }

    var errorDescription: String? { description }
}

/// Represents the contents of a `slice-plugin.json` file.
///
/// - `sources`: Optional list of Slice files or directories containing Slice files. Paths are relative to the
///   directory containing the `slice-plugin.json` file. Slice files declared directly in the target's source files
///   are always included and do not need to be listed here.
///
/// - `search_paths`: Optional list of directories to add as `-I` search paths when invoking `slice2swift`.
///   These paths are also relative to the config file location. Note: The Ice Slice directory is automatically
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
    /// Note: The Ice Slice directory is automatically included.
    var search_paths: [String]?
}

/// The CompileSlicePlugin for SwiftPM compiles Ice Slice files to Swift files using the `slice2swift` compiler.
/// The `slice2swift` compilation can be configured using a `slice-plugin.json` file in the target's source files.
/// By default the plugin will compile all `.ice` files in the target's source files.
///
/// The Ice Slice directory is automatically added to the search path, so you don't need to include it in
/// `search_paths`. This allows Slice files to import Ice definitions (e.g., `#include <Ice/Identity.ice>`)
/// without additional configuration.
@main
struct CompileSlicePlugin {

    /// The name of the configuration file.
    private static let configFileName = "slice-plugin.json"

    /// The Ice Slice directory, derived from this plugin's source file location.
    /// Path: CompileSlicePlugin.swift -> CompileSlice -> Plugins -> swift -> (ice root) -> slice
    private static let iceSliceDir: URL = {
        var url = URL(fileURLWithPath: #filePath)
        for _ in 0..<4 {
            url.deleteLastPathComponent()
        }
        url.append(path: "slice")
        return url
    }()

    private func createBuildCommands(
        outputDir: URL,
        inputFiles: FileList,
        slice2swift: URL
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

        // Decode config and apply additional sources and search paths.
        var searchPathDirs: [URL] = []
        if let configFileURL = configFileURL {
            let configData = try Data(contentsOf: configFileURL)
            let config = try JSONDecoder().decode(Config.self, from: configData)

            // Add additional Slice files from config.sources. The files are relative to the config file location.
            let baseDirectory = configFileURL.deletingLastPathComponent()
            for source in config.sources ?? [] {
                let sourceFileOrDirectory = baseDirectory.appending(path: source)
                if sourceFileOrDirectory.pathExtension == "ice" {
                    sliceSources.append(sourceFileOrDirectory)
                } else {
                    // Standardized because contentsOfDirectory(at:) does not follow a symbolic link.
                    sliceSources.append(
                        contentsOf: try FileManager.default.contentsOfDirectory(
                            at: sourceFileOrDirectory.standardizedFileURL,
                            includingPropertiesForKeys: nil
                        ).filter { $0.pathExtension == "ice" })
                }
            }

            // Add additional search paths from config.search_paths.
            // These paths are relative to the config file location.
            for path in config.search_paths ?? [] {
                searchPathDirs.append(baseDirectory.appending(path: path))
            }
        }

        // Add the Ice Slice directory last, so user-provided paths take precedence.
        guard FileManager.default.fileExists(atPath: Self.iceSliceDir.appending(path: "Ice/Identity.ice").path) else {
            throw PluginError.missingIceSliceFiles(Self.iceSliceDir.path)
        }
        searchPathDirs.append(Self.iceSliceDir)

        // slice2swift resolves the paths it reports, so match it before looking dependencies up.
        sliceSources = sliceSources.map { $0.standardizedFileURL }

        let includeArguments = searchPathDirs.map { "-I\($0.path)" }

        // Ask the compiler which files each source includes, so that editing an included file
        // regenerates every file that includes it.
        let dependencies = try Self.sliceDependencies(
            slice2swift: slice2swift,
            includeArguments: includeArguments,
            sources: sliceSources
        )

        // A source missing from the report would silently lose its dependencies, so treat it as an error
        // rather than generating Swift code that later goes stale.
        if let dependencies {
            for sliceSource in sliceSources where dependencies[sliceSource.path] == nil {
                throw PluginError.dependencyScanFailed("nothing was reported for '\(sliceSource.path)'")
            }
        }

        // Create the build commands for each Slice file.
        return sliceSources.map { sliceSource in
            let outputFile = outputDir.appending(path: sliceSource.lastPathComponent)
                .deletingPathExtension()
                .appendingPathExtension("swift")

            return .buildCommand(
                displayName: "Compile Slice \(sliceSource.lastPathComponent)",
                executable: slice2swift,
                arguments: includeArguments + ["--output-dir", outputDir.path, sliceSource.path],
                // The build system re-runs this command when any declared input changes, so it must see
                // everything slice2swift reads: the compiler, the config file and the included Slice files.
                inputFiles: [sliceSource, slice2swift]
                    + (dependencies?[sliceSource.path] ?? [])
                    + (configFileURL.map { [$0] } ?? []),
                outputFiles: [URL(fileURLWithPath: outputFile.path)]
            )
        }
    }

    /// Runs `slice2swift --depend-xml` to find the Slice files each source includes, directly or
    /// transitively. Returns nil when the compiler rejects the Slice files: the build commands then report
    /// those errors, and the next successful run restores the dependencies.
    private static func sliceDependencies(
        slice2swift: URL,
        includeArguments: [String],
        sources: [URL]
    ) throws -> [String: [URL]]? {
        let process = Process()
        process.executableURL = slice2swift
        process.arguments = includeArguments + ["--depend-xml"] + sources.map { $0.path }

        let standardOutput = Pipe()
        process.standardOutput = standardOutput
        process.standardError = FileHandle.nullDevice

        do {
            try process.run()
        } catch {
            throw PluginError.dependencyScanFailed("'\(slice2swift.path)' could not be run")
        }

        // Read before waiting so a large dependency graph cannot fill the pipe and deadlock.
        let output = standardOutput.fileHandleForReading.readDataToEndOfFile()
        process.waitUntilExit()

        // The compiler reports nothing when any of the Slice files fails to parse.
        guard process.terminationStatus == 0 else {
            return nil
        }

        guard let dependencies = DependencyParser().parse(output) else {
            throw PluginError.dependencyScanFailed("the output of '--depend-xml' could not be parsed")
        }
        return dependencies
    }
}

/// Parses the output of `slice2swift --depend-xml`: one `<source>` element per Slice file, holding a
/// `<dependsOn>` element per included file. The compiler flattens transitive includes for us.
private final class DependencyParser: NSObject, XMLParserDelegate {
    private var dependencies: [String: [URL]] = [:]
    private var currentSource: String?

    func parse(_ data: Data) -> [String: [URL]]? {
        let parser = XMLParser(data: data)
        parser.delegate = self
        return parser.parse() ? dependencies : nil
    }

    func parser(
        _ parser: XMLParser,
        didStartElement elementName: String,
        namespaceURI: String?,
        qualifiedName: String?,
        attributes: [String: String]
    ) {
        guard let name = attributes["name"] else {
            return
        }
        let url = URL(fileURLWithPath: name).standardizedFileURL

        switch elementName {
        case "source":
            currentSource = url.path
            dependencies[url.path] = []
        case "dependsOn":
            if let currentSource {
                dependencies[currentSource]?.append(url)
            }
        default:
            break
        }
    }

    func parser(
        _ parser: XMLParser,
        didEndElement elementName: String,
        namespaceURI: String?,
        qualifiedName: String?
    ) {
        if elementName == "source" {
            currentSource = nil
        }
    }
}
