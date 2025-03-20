// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

import org.gradle.api.DefaultTask
import org.gradle.api.GradleException
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.logging.Logger
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.w3c.dom.Element
import java.io.File
import java.nio.file.Files
import javax.inject.Inject
import javax.xml.parsers.DocumentBuilderFactory
import javax.xml.transform.OutputKeys
import javax.xml.transform.TransformerFactory
import javax.xml.transform.dom.DOMSource
import javax.xml.transform.stream.StreamResult

/**
 * Task for compiling Slice files using the Slice-to-Java compiler (`slice2java`).
 *
 * This task processes Slice files (`.ice`) and generates corresponding Java source files.
 */
abstract class SliceTask @Inject constructor(objects: ObjectFactory) : DefaultTask() {
    /**
     * Additional arguments to pass to the `slice2java` compiler.
     */
    @get:Input
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /**
     * Directories to search for Slice files when compiling with `slice2java`.
     *
     * These directories are passed to the compiler using the `-I` flag.
     */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val includeSearchPath: ConfigurableFileCollection

    /**
     * The Slice source files to be compiled.
     * These `.ice` files are processed by the `slice2java` compiler to generate Java source files.
     */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val inputFiles: ConfigurableFileCollection

    /**
     * The output directory where generated Java files will be written.
     *
     * This directory is populated by the `slice2java` compiler during execution.
     */
    @get:OutputDirectory
    abstract val output: DirectoryProperty

    /**
     * The base name used to determine the output directory for the generated files.
     */
    @get:Input
    abstract val outputBaseName: Property<String>

    /**
     * The path to the Ice tools directory, used to locate the `slice2java` compiler.
     *
     * This directory must contain the `slice2java` executable for the current platform.
     */
    @get:Input
    abstract val toolsPath: Property<String>

    /**
     * Path to a file storing the arguments used in the previous compilation.
     *
     * This file records the compiler options passed to `slice2java`, allowing the detection
     * of option changes for incremental build checks.
     */
    private val argsFile: File
        get() = File(output.get().asFile, "args.txt")

    /**
     * Path to a file storing the output of the `--depend-xml` option from the previous compilation.
     *
     * This file contains dependency information that allows incremental builds.
     */
    private val dependXmlFile: File
        get() = File(output.get().asFile, "depend.xml")

    /**
     * Path to a file storing the output of the `--list-generated` option from the previous compilation.
     *
     * This file helps track which files were generated in the last execution.
     */
    private val generatedXmlFile: File
        get() = File(output.get().asFile, "generated.xml")

    private val workingDir: File = project.projectDir
    private val taskLogger: Logger = project.logger

    init {
        // Ensure the task always runs, preventing Gradle from marking it as UP-TO-DATE. This allow us to check for
        // changes in the Slice files and dependencies.
        outputs.upToDateWhen { false }

        // Set default output directory to `build/generated/source/slice/<outputBaseName>`
        output.convention(
            outputBaseName.flatMap { project.layout.buildDirectory.dir("generated/source/slice/$it") },
        )
    }

    /**
     * Compiles Slice files into Java source files using `slice2java`, with incremental compilation support.
     *
     * This task:
     * - Detects changes in Slice files and their dependencies
     * - Runs `slice2java` only on changed files to minimize recompilation
     * - Tracks generated files and removes stale files from previous builds
     * - Updates dependency information to ensure correct incremental builds
     *
     * If no Slice files are found, the output directory is cleaned.
     * If compilation arguments change, a full recompilation is triggered.
     */
    @TaskAction
    fun compileSlice() {
        val sliceFiles = inputFiles.files.toList()
        val outputDir = output.get().asFile

        if (sliceFiles.isEmpty()) {
            // No Slice files to compile. Ensure the output directory is clean by deleting any stale files
            outputDir.deleteRecursively()
            return
        }

        // Ensure the output directory exists
        outputDir.mkdirs()

        // Load arguments from the previous compilation if available
        val previousArgs = if (argsFile.exists()) argsFile.readLines() else emptyList()

        // Compute the arguments for the current compilation
        val currentArgs = getSlice2JavaCommand()

        if (previousArgs != currentArgs) {
            // If the compilation arguments have changed, force a full recompilation
            outputDir.deleteRecursively()
            outputDir.mkdirs()

            // Save the new arguments for future incremental builds
            argsFile.writeText(currentArgs.joinToString("\n"))
        }

        // Load dependency information from the previous compilation if available
        val dependencies = if (dependXmlFile.exists() && dependXmlFile.length() > 0) {
            parseDependencies(dependXmlFile.readText())
        } else {
            emptyMap()
        }

        // Generate updated dependency information by running `slice2java --depend-xml`
        generateDependencies(sliceFiles)

        // Load the list of generated files from `generated.xml`, corresponding to the previous compilation
        val previousGeneratedFiles = loadPreviousGeneratedFiles()

        // Determine which files need to be recompiled based on dependencies
        val changedFiles = getChangedFiles(sliceFiles, dependencies, previousGeneratedFiles, File(getSlice2JavaPath()))

        var newGeneratedFiles: Map<File, List<String>> = emptyMap()
        if (changedFiles.isNotEmpty()) {
            // Compile changed Slice files with `slice2java` and track the generated files
            val command = getSlice2JavaCommand(changedFiles, listOf("--list-generated"))
            val process = ProcessBuilder(command)
                .directory(workingDir)
                .redirectErrorStream(false) // Keep stdout and stderr separate
                .start()

            val stdoutReader = process.inputStream.bufferedReader()
            val stderrReader = process.errorStream.bufferedReader()

            val output = stdoutReader.readText()
            val errors = stderrReader.readText()
            val exitCode = process.waitFor()

            printErrorsAndWarnings(errors.lineSequence(), taskLogger)
            if (exitCode != 0) {
                throw GradleException("Command failed: $command (exit code: $exitCode)")
            }
            newGeneratedFiles = parseGeneratedFiles(output)
        }

        // Merge the new generated files with the previous compilation results.
        // This ensures that:
        // - Updated Slice files have their generated Java files tracked
        // - Unchanged Slice files retain their previous generated Java files
        // - Removed Slice files have their generated Java files deleted
        val mergedGeneratedFiles = mergeGeneratedFiles(previousGeneratedFiles, newGeneratedFiles, sliceFiles)
        saveGeneratedFiles(mergedGeneratedFiles)

        // Remove stale files from the output directory. These are files that were previously generated
        // but are no longer present in the latest compilation results.
        val generatedFilesSet = mergedGeneratedFiles.values.flatten().map { File(it).canonicalFile }.toSet()
        deleteStaleFiles(outputDir, generatedFilesSet)
    }

    private fun printErrorsAndWarnings(lines: Sequence<String>, logger: Logger) {
        val warningRegex = Regex("""(.*):[0-9]+:\s+warning:(.*)""")

        lines.map { it.trim() }
            .filter { it.isNotBlank() }
            .forEach { line ->
                when {
                    warningRegex.matches(line) -> logger.warn(line)
                    else -> logger.error(line) // Treat everything else as an error
                }
            }
    }

    /**
     * Parses the XML output of `slice2java --list-generated` to extract the generated files for each Slice file.
     *
     * @param xmlOutput The XML output of `slice2java --list-generated`.
     * @return A map where keys are source `.ice` files and values are lists of generated Java files.
     */
    private fun parseGeneratedFiles(xmlOutput: String): Map<File, List<String>> {
        val builder = DocumentBuilderFactory.newInstance().newDocumentBuilder()
        val doc = builder.parse(xmlOutput.byteInputStream())

        return doc.getElementsByTagName("source")
            .toSequence()
            .mapNotNull { it as? Element }
            .associate { sourceElement ->
                val sourceFile = File(sourceElement.getAttribute("name")).canonicalFile
                val generatedFiles = sourceElement.getElementsByTagName("file")
                    .toSequence()
                    .mapNotNull { (it as? Element)?.getAttribute("name") }
                    .toList()
                sourceFile to generatedFiles
            }
    }

    /**
     * Generates the dependencies between Slice files using `slice2java --depend-xml`.
     *
     * The output is saved to `depend.xml`.
     *
     * @param sliceFiles The list of Slice files to generate dependencies for.
     */
    private fun generateDependencies(sliceFiles: List<File>) {
        val command = getSlice2JavaCommand(sliceFiles, listOf("--depend-xml"))
        val process = ProcessBuilder(command)
            .directory(workingDir)
            .redirectOutput(dependXmlFile) // Write directly to depend.xml
            .redirectErrorStream(false)
            .start()

        val stderrReader = process.errorStream.bufferedReader()
        val errorOutput = stderrReader.readText()
        val exitCode = process.waitFor()
        printErrorsAndWarnings(errorOutput.lineSequence(), taskLogger)
        if (exitCode != 0) {
            throw GradleException("Command failed: $command (exit code: $exitCode)")
        }
    }

    /**
     * Parses the XML output of `slice2java --depend-xml` to extract dependencies between Slice files.
     *
     * @param xmlOutput The XML output of `slice2java --depend-xml`.
     * @return A map where keys are source `.ice` files and values are lists of dependent `.ice` files.
     */
    private fun parseDependencies(xmlOutput: String): Map<File, List<File>> {
        val builder = DocumentBuilderFactory.newInstance().newDocumentBuilder()
        val doc = builder.parse(xmlOutput.byteInputStream())

        return doc.getElementsByTagName("source")
            .toSequence()
            .mapNotNull { it as? Element }
            .associate { sourceElement ->
                val sourceFile = File(sourceElement.getAttribute("name")).canonicalFile
                val dependencies = sourceElement.getElementsByTagName("dependsOn")
                    .toSequence()
                    .mapNotNull { (it as? Element)?.getAttribute("name") }
                    .map { File(it).canonicalFile }
                    .toList()
                sourceFile to dependencies
            }
    }

    /**
     * Returns the path to the `slice2java` compiler executable.
     *
     * This method ensures that `slice2java` exists in `toolsPath` and throws an exception if it is missing.
     *
     * @return The absolute path to the `slice2java` executable.
     * @throws GradleException if `slice2java` does not exist in the expected location.
     */
    private fun getSlice2JavaPath(): String {
        val slice2javaExe = if (SliceToolsUtil.isWindows()) "slice2java.exe" else "slice2java"

        val path = toolsPath.orNull?.let { "$it/$slice2javaExe" }
            ?: throw IllegalStateException("toolsPath is unexpectedly unset. This indicates a logic bug in the plugin.")

        val slice2javaFile = File(path)
        if (!slice2javaFile.exists() || !slice2javaFile.isFile) {
            throw GradleException("Required compiler `$slice2javaExe` not found in toolsPath: ${toolsPath.get()}")
        }

        return path
    }

    /**
     * Constructs the command to execute `slice2java` with the specified Slice files and arguments.
     *
     * This method builds the full command-line invocation for `slice2java`, including:
     * - The compiler executable path
     * - Include search paths (`-I` options)
     * - The output directory (`--output-dir`)
     * - Default compiler arguments
     * - Additional arguments provided by the caller
     * - The list of Slice files to compile
     *
     * @param sliceFiles The list of `.ice` Slice files to compile. Defaults to an empty list.
     * @param additionalArgs Additional command-line arguments to pass to `slice2java`. Defaults to an empty list.
     * @return A list of strings representing the full `slice2java` command.
     */
    private fun getSlice2JavaCommand(sliceFiles: List<File> = emptyList(), additionalArgs: List<String> = emptyList()): List<String> {
        val includeArgs = includeSearchPath.files.flatMap { listOf("-I", it.absolutePath) }

        val slice2javaPath = getSlice2JavaPath()

        val command = listOf(slice2javaPath) +
            includeArgs +
            listOf("--output-dir", output.get().asFile.absolutePath) +
            compilerArgs.getOrElse(emptyList()) +
            additionalArgs +
            sliceFiles.map { it.absolutePath }
        return command
    }

    /**
     * Determines which Slice files need to be recompiled based on:
     *
     * 1. New files: If a file is not in `dependencies`, it must be compiled.
     * 2. Dependencies: If a dependency file has been modified after the Slice file.
     * 3. Generated files: If any generated file is missing or older than its corresponding source file.
     * 4. Slice2Java: If the Slice2Java compiler has been updated.
     *
     * @param sliceFiles The list of all Slice files in the project.
     * @param dependencies A map of Slice files to their dependencies.
     * @param generatedFilesMap A map of Slice files to their corresponding generated Java files.
     * @return A list of Slice files that require recompilation.
     */
    private fun getChangedFiles(
        sliceFiles: List<File>,
        dependencies: Map<File, List<File>>,
        generatedFilesMap: Map<File, List<String>>,
        slice2java: File,
    ): List<File> {
        val changedFiles = mutableSetOf<File>()

        val slice2javaLastModified = slice2java.lastModified()

        for (sliceFile in sliceFiles) {
            if (!sliceFile.exists()) {
                // Skip deleted files
                continue
            }

            val sliceLastModified = sliceFile.lastModified()

            // Get the list of generated files for this Slice file
            val generatedFiles = generatedFilesMap[sliceFile].orEmpty()

            // Get the first generated file for this Slice file
            val firstGeneratedFile = generatedFiles
                .map { File(it) }
                .firstOrNull { it.exists() }

            // If no generated files exist, force recompilation
            if (firstGeneratedFile == null) {
                logger.info("No generated files found for Slice file: $sliceFile (Recompiling)")
                changedFiles.add(sliceFile)
                continue
            }

            // Use the timestamp of the first generated file as the last compilation time
            val lastCompiledTimestamp = firstGeneratedFile.lastModified()

            if (lastCompiledTimestamp < slice2javaLastModified) {
                logger.info("Slice2Java compiler has been updated since last compilation: $slice2java")
                changedFiles.add(sliceFile)
                continue
            }

            // Check if any generated Java file is missing or outdated
            for (generatedFilePath in generatedFiles) {
                val generatedFile = File(generatedFilePath) // Ensure correct absolute path

                // Recompile if the generated file is missing
                if (!generatedFile.exists()) {
                    logger.info("Missing generated file: $generatedFile")
                    changedFiles.add(sliceFile)
                    break
                }

                // Recompile if the generated file is older than the Slice file
                if (generatedFile.lastModified() < sliceLastModified) {
                    logger.info("Outdated generated file: $generatedFile")
                    changedFiles.add(sliceFile)
                    break
                }
            }

            // If the file is not in `dependencies`, assume it's new and must be compiled
            if (sliceFile !in dependencies) {
                logger.info("New Slice file detected: $sliceFile (No previous dependencies found)")
                changedFiles.add(sliceFile)
                continue
            }

            // Retrieve dependencies (could be empty)
            val sliceDependencies = dependencies[sliceFile].orEmpty()

            // Check if any dependency is newer than the first generated file
            val changedDependency = sliceDependencies.find { it.exists() && it.lastModified() > lastCompiledTimestamp }
            if (changedDependency != null) {
                logger.info("Dependency changed for Slice file: $sliceFile (Updated dependency: $changedDependency, last compiled at $lastCompiledTimestamp)")
                changedFiles.add(sliceFile)
                continue
            }
        }

        return changedFiles.toList()
    }

    /**
     * Deletes stale generated files from the task's output directory.
     *
     * This ensures that only files listed in `--list-generated` remain in the output directory.
     *
     * @param outputDir The directory where generated files are stored.
     * @param generatedFilesSet A set containing the absolute paths of valid generated files (from `--list-generated`).
     */
    private fun deleteStaleFiles(outputDir: File, generatedFilesSet: Set<File>) {
        if (!outputDir.exists()) return

        // Delete any file in `outputDir` that is NOT in `generatedFilesSet`
        outputDir.walkTopDown()
            .filter { it.isFile && it.extension == "java" && it !in generatedFilesSet }
            .forEach { file ->
                file.delete()
            }
    }

    /**
     * Loads the previously generated files from `generated.xml`.
     *
     * If `generated.xml` does not exist, returns an empty map.
     *
     * @return A map where keys are `.ice` source files and values are lists of generated Java files.
     */
    private fun loadPreviousGeneratedFiles(): Map<File, List<String>> {
        if (!generatedXmlFile.exists()) return emptyMap()
        return parseGeneratedFiles(generatedXmlFile.readText())
    }

    /**
     * Merges newly generated files with the previous compilation results, ensuring stale files are removed.
     *
     * This function updates the mapping of Slice files to their corresponding generated Java files. It:
     * - Replaces previous entries with newly generated files, ensuring only the latest files are kept.
     * - Removes entries for Slice files that no longer exist, preventing stale files from persisting.
     *
     * @param previousGeneratedFiles A map of Slice source files to their previously generated Java files.
     * @param newGeneratedFiles A map of Slice source files to their newly generated Java files.
     * @param sliceFiles The current list of Slice source files included in the task `inputFiles`.
     * @return A merged map containing only up-to-date generated files.
     */
    private fun mergeGeneratedFiles(
        previousGeneratedFiles: Map<File, List<String>>,
        newGeneratedFiles: Map<File, List<String>>,
        sliceFiles: List<File>,
    ): Map<File, List<String>> {
        val mergedFiles = previousGeneratedFiles.toMutableMap()

        // Replace previous entries with newly generated files to ensure the latest versions are kept.
        newGeneratedFiles.forEach { (sliceFile, files) ->
            mergedFiles[sliceFile] = files
        }

        // Remove any entries that no longer exist in `sliceFiles`, ensuring stale files are not retained.
        mergedFiles.keys.retainAll(sliceFiles)

        return mergedFiles
    }

    /**
     * Saves the generated files map to `generated.xml`, or deletes the file if empty.
     *
     * @param generatedFilesMap The map of Slice source files to their generated Java files.
     */
    private fun saveGeneratedFiles(generatedFilesMap: Map<File, List<String>>) {
        val file = generatedXmlFile

        if (generatedFilesMap.isEmpty()) {
            Files.deleteIfExists(file.toPath())
            return
        }

        val doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument()
        val rootElement = doc.createElement("generated").also { doc.appendChild(it) }

        generatedFilesMap.forEach { (sliceFile, files) ->
            val sourceElement = doc.createElement("source").apply {
                setAttribute("name", sliceFile.absolutePath)
                files.forEach { file ->
                    appendChild(doc.createElement("file").apply { setAttribute("name", file) })
                }
            }
            rootElement.appendChild(sourceElement)
        }

        TransformerFactory.newInstance().newTransformer().apply {
            setOutputProperty(OutputKeys.INDENT, "yes")
            setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2")
            transform(DOMSource(doc), StreamResult(file))
        }
    }
}
