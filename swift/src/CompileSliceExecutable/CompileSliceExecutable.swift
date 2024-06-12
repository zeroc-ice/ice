import Foundation
import ArgumentParser

enum CompileError: Error {
    case missingIceSliceFiles(String)
    case sliceCompilerError(Int32)
    
    var description: String {
        switch self {
            case let .missingIceSliceFiles(path):
                return "The Ice slice files are missing. Expected location: `\(path)`"
            case let .sliceCompilerError(status):
                return "The Slice compiler failed with status code: \(status)"
        }
    }
}

 @main
 struct PluginExecutable: ParsableCommand {
    @Argument(help: "Path to the slice compiler executable")
    var sliceCompilerPath: String

     @Argument(help: "Slice file to compile")
     var sliceFile: String

     @Argument(help: "Output directory")
     var outputDir: String

     func run() throws {
         // TODO: is there a better way to find this bundle path
         let iceSliceFilePath = Bundle.main.bundlePath.appending("/ice_CompileSliceExecutable.bundle")
         
         if !FileManager.default.fileExists(atPath: iceSliceFilePath) {
             throw CompileError.missingIceSliceFiles(iceSliceFilePath)
         }

         
        let process = Process()
        // process.standardOutput = stdout
        // process.standardError = stderr
        process.executableURL = URL(fileURLWithPath: sliceCompilerPath)
        process.arguments = [
            sliceFile,
            "--output-dir",
            outputDir
        ]

        try process.run()

        process.waitUntilExit()

         // TODO: figure out how to return an error message to the console
        if (process.terminationStatus != 0) {
            throw CompileError.sliceCompilerError(process.terminationStatus)
        }
     }
 }
