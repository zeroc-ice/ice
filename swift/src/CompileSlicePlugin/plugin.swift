import PackagePlugin
import Foundation

enum PluginError: Error {
    case invalidTarget(Target)
    case missingCompiler(String)
    case missingConfigFile(String, String)

    var description: String {
        switch self {
            case let .invalidTarget(target):
                return "Expected a SwiftSourceModuleTarget but got '\(type(of: target))'."
            case let .missingCompiler(path):
                return "Missing slice compiler: '\(path)'."
            case let .missingConfigFile(path, target):
                return "Missing config file '\(path)` for target `\(target)`. '. This file must be included in your sources."
        }
    }
}

public func findIceSlicePath(_ context: PluginContext) throws -> String {
    let slice = context.package.directory.appending("slice")
    guard FileManager.default.fileExists(atPath: slice.string) else {
        throw PluginError.missingCompiler("Ice Slice directory not found at: `\(slice)`")
    }
    return slice.string
}

/// Represents the contents of a `slice-plugin.json` file
struct Config: Codable {
    var sources: [String]
}

@main
struct CompileSlicePlugin: BuildToolPlugin {

    static let configFileName = "slice-plugin.json"

    func createBuildCommands(context: PluginContext, target: Target) async throws -> [Command] {
        guard let sourceModuleTarget = target as? SwiftSourceModuleTarget else {
                throw PluginError.invalidTarget(target)
        }

        let sourceFiles = sourceModuleTarget.sourceFiles
        guard let configFilePath = sourceFiles.first(
            where: {
                $0.path.lastComponent == Self.configFileName
                }
        )?.path else {
            throw PluginError.missingConfigFile(Self.configFileName, target.name)
        }

        let data = try Data(contentsOf: URL(fileURLWithPath: configFilePath.string))
        let config = try JSONDecoder().decode(Config.self, from: data)

        let iceSlicePath = try findIceSlicePath(context)
        let slice2swift = try context.tool(named: "slice2swift").path

        // Find the Ice Slice files for the corresponding Swift target
        let fm = FileManager.default

        let sources = try config.sources.map{ source in
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

        // Create a build command for each Slice file
        return sources.map { (sliceFile) -> Command in
            // Absolute path to the input Slice file
            let inputFile = sliceFile
            // Absolute path to the output Slice file
            let outputFile = Path(URL(fileURLWithPath: outputDir.appending(sliceFile.lastComponent).string).deletingPathExtension().appendingPathExtension("swift").relativePath)

            return .buildCommand(
                displayName: "slice2swift \(sliceFile) -> \(outputDir)",
                executable: slice2swift,
                arguments: [
                    "-I\(iceSlicePath)",
                    "--output-dir",
                    outputDir,
                    inputFile
                ],
                outputFiles: [outputFile]
            )
        }
    }
}
