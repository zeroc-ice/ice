// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.DefaultTask
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import java.io.File
import javax.inject.Inject
import javax.xml.parsers.DocumentBuilder
import javax.xml.parsers.DocumentBuilderFactory
import javax.xml.transform.OutputKeys
import javax.xml.transform.TransformerFactory
import javax.xml.transform.dom.DOMSource
import javax.xml.transform.stream.StreamResult

/**
 * Task for compiling Slice files using the Slice-to-Java compiler (`slice2java`).
 *
 * This task processes Slice files (`.ice`) and generates corresponding Java source files. The generated
 * files are written to a specified output directory.
 */
abstract class SliceTask @Inject constructor(objects: ObjectFactory) : DefaultTask() {

    /** The Slice source files to compile */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val slice: ConfigurableFileCollection

    /** The base name used to compute the output directory. */
    @get:Input
    abstract val outputBaseName: Property<String>

    /** 
     * Directories to search for Slice files when compiling with `slice2java`. 
     * These directories are passed to the compiler using the `-I` flag.
     */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val includeSearchPath: ConfigurableFileCollection

    /** 
     * Additional arguments to pass to the `slice2java` compiler.
     */
    @get:Input
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /** 
     * The path to the Ice tools directory, used to locate the `slice2java` compiler. 
     * This directory must contain the `slice2java` executable for the task to run successfully.
     */
    @get:Input
    abstract val toolsPath: Property<String>

    /** 
     * The output directory where generated Java files will be written. 
     * This directory is populated by the `slice2java` compiler during execution.
     */
    @get:OutputDirectory
    abstract val output: DirectoryProperty

    /** Path to a file storing the output of --list-generated from the previous compilation */
    private val generatedXmlFile: File
        get() = File(output.get().asFile, "generated.xml")

    /** Path to a file storing the output of --depend-xml from the previous compilation */
    private val dependXmlFile: File
        get() = File(output.get().asFile, "depend.xml")

    /** Path to a file storing the arguments used in the previous compilation */
    private val argsFile: File
        get() = File(output.get().asFile, "args.txt")

    init {
        // Ensure the task always runs, preventing Gradle from marking it as UP-TO-DATE. This allow us to check for
        // changes in the Slice files and dependencies.
        outputs.upToDateWhen { false }

        // Set default output directory to `build/generated/source/slice/<outputBaseName>`
        output.convention(
            outputBaseName.flatMap { project.layout.buildDirectory.dir("generated/source/slice/$it") },
        )
    }

    @TaskAction
    fun compileSlice() {
        val outputDir = output.get().asFile
        outputDir.mkdirs()

        val sliceFiles = slice.files.toList()

        if (sliceFiles.isEmpty()) {
            // No Slice files to compile, we can safely delete the output directory to ensure
            // there are no stale files left over from a previous compilation.
            outputDir.deleteRecursively()
            return
        }

        // Load arguments from previous compilation if available
        val previousArgs = if (argsFile.exists()) argsFile.readLines() else emptyList()

        // Compute the arguments for the current compilation
        val currentArgs = getSlice2JavaCommand()

        if (previousArgs != currentArgs) {
            // If the arguments have changed, force recompilation by removing all generated files
            outputDir.deleteRecursively()
            outputDir.mkdirs()
            // Save the new arguments
            argsFile.writeText(currentArgs.joinToString("\n"))
        }

        // Parse dependencies from previous compilation if available
        val dependencies = if (dependXmlFile.exists() && dependXmlFile.length() > 0) {
            parseDependencies(dependXmlFile.readText())
        } else {
            emptyMap()
        }

        // Run `--depend-xml` on all Slice files to update dependencies
        generateDependencies(sliceFiles)

        // Load the list of generated files from `generated.xml`, corresponding to the previous compilation
        val previousGeneratedFiles = loadPreviousGeneratedFiles()

        // Determine which files must be recompiled
        val changedFiles = getChangedFiles(sliceFiles, dependencies, previousGeneratedFiles, File(getSlice2JavaPath()))

        logger.lifecycle("Compiling Slice files: $changedFiles")

        var newGeneratedFiles: Map<File, List<String>> = emptyMap()
        if (!changedFiles.isEmpty()) {
            val command = getSlice2JavaCommand(changedFiles, listOf("--list-generated"))
            val process = ProcessBuilder(command)
                .directory(project.projectDir)
                .redirectErrorStream(false) // Keep stdout and stderr separate
                .start()

            val stdoutReader = process.inputStream.bufferedReader()
            val stderrReader = process.errorStream.bufferedReader()

            val output = stdoutReader.readText()
            val errors = stderrReader.readText()
            val exitCode = process.waitFor()

            if (exitCode != 0) {
                logger.error("Failed to compile Slice files: $errors")
            } else if (errors.isNotBlank()) {
                logger.warn("Warnings while compiling Slice files: $errors")
            }

            newGeneratedFiles = parseGeneratedFiles(output)
        }

        // Compute the list of generated files for the current compilation, updating the list of generated files
        // for each updated Slice file, keeping previous generated files for unchanged Slice files, and removing
        // generated files for removed Slice files.
        val mergedGeneratedFiles = mergeGeneratedFiles(previousGeneratedFiles, newGeneratedFiles, sliceFiles)
        saveGeneratedFiles(mergedGeneratedFiles)

        // Remove stale files from the output directory, this are files that are no longer in the generated files list.
        val generatedFilesSet = mergedGeneratedFiles.values.flatten().map { File(it).canonicalFile }.toSet()
        deleteStaleFiles(outputDir, generatedFilesSet)
    }

    /**
     * Parses the XML output of `slice2java --list-generated` to extract the generated files for each Slice file.
     * Returns a map of source files to their generated files.
     *
     * The output is a list of `<source>` elements, each with a `name` attribute representing the source file, and
     * zero or more `<file>` elements representing the generated source files.
     *
     * @param xmlOutput The XML output of `slice2java --list-generated`.
     * @return A map where keys are source `.ice` files and values are lists of generated Java files.
     */
    private fun parseGeneratedFiles(xmlOutput: String): Map<File, List<String>> {
        val generatedFilesMap = mutableMapOf<File, MutableList<String>>()
        val factory = DocumentBuilderFactory.newInstance()
        val builder = factory.newDocumentBuilder()
        val doc = builder.parse(xmlOutput.byteInputStream())

        val sources = doc.getElementsByTagName("source")
        for (i in 0 until sources.length) {
            val sourceNode = sources.item(i)
            val sourceNameAttr = sourceNode.attributes.getNamedItem("name") ?: continue
            val sourceFile = File(sourceNameAttr.nodeValue).canonicalFile

            val generatedFiles = mutableListOf<String>()
            val childNodes = sourceNode.childNodes
            for (j in 0 until childNodes.length) {
                val fileNode = childNodes.item(j)
                if (fileNode.nodeName == "file") {
                    val fileNameAttr = fileNode.attributes.getNamedItem("name")
                    if (fileNameAttr != null) {
                        generatedFiles.add(fileNameAttr.nodeValue)
                    }
                }
            }

            generatedFilesMap[sourceFile] = generatedFiles
        }
        return generatedFilesMap
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
            .directory(project.projectDir)
            .redirectOutput(dependXmlFile) // Write directly to depend.xml
            .redirectErrorStream(false)
            .start()

        val stderrReader = process.errorStream.bufferedReader()
        val exitCode = process.waitFor()

        if (exitCode != 0) {
            val errorOutput = stderrReader.readText() // Read stderr only when there's an error
            logger.error(errorOutput)
        }
    }

    /**
     * Parses the XML output of `slice2java --depend-xml` to extract the dependencies between Slice files.
     * And returns a map of source files to their dependencies.
     *
     * The output is a list of <source> elements, each with a name attribute representing the source file, and
     * zero or more <dependsOn> elements representing the dependencies of the source file.
     *
     * @param xmlOutput The XML output of `slice2java --depend-xml`.
     * @return A map of source files to their dependencies.
     */
    private fun parseDependencies(xmlOutput: String): Map<File, List<File>> {
        val dependencyMap = mutableMapOf<File, List<File>>()
        val factory = DocumentBuilderFactory.newInstance()
        val builder = factory.newDocumentBuilder()
        val doc = builder.parse(xmlOutput.byteInputStream())

        val sources = doc.getElementsByTagName("source")
        for (i in 0 until sources.length) {
            val source = sources.item(i)
            val sourceFile = File(source.attributes.getNamedItem("name").nodeValue)
            val dependsOnFiles = mutableListOf<File>()
            val dependsOnNodes = source.childNodes
            for (j in 0 until dependsOnNodes.length) {
                val dependency = dependsOnNodes.item(j)
                if (dependency.nodeName == "dependsOn") {
                    val dependencyPath = dependency.attributes.getNamedItem("name").nodeValue
                    dependsOnFiles.add(File(dependencyPath))
                }
            }
            dependencyMap[sourceFile] = dependsOnFiles
        }
        return dependencyMap
    }

    /**
     * Returns the path to the `slice2java` compiler executable.
     *
     * TODO: throw if it doesn't exist and don't fallback to PATH
     */
    private fun getSlice2JavaPath(): String {
        // Determine the location of `slice2java`
        val slice2javaExe = when {
            SliceToolsUtil.isWindows() -> "slice2java.exe"
            else -> "slice2java"
        }
        return toolsPath.orNull?.let { "$it/$slice2javaExe" } ?: slice2javaExe
    }

    /**
     * Returns the command to run `slice2java` with the given Slice files and additional arguments.
     *
     * @param sliceFiles The list of Slice files to compile.
     * @param additionalArgs Additional arguments to pass to `slice2java`.
     * @return A list of strings representing the command to run.
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
                continue // Skip deleted files
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
                logger.lifecycle("No generated files found for Slice file: $sliceFile (Recompiling)")
                changedFiles.add(sliceFile)
                continue
            }

            // Use the timestamp of the first generated file as the last compilation time
            val lastCompiledTimestamp = firstGeneratedFile.lastModified()

            if (lastCompiledTimestamp < slice2javaLastModified) {
                logger.lifecycle("Slice2Java compiler has been updated since last compilation: $slice2java")
                changedFiles.add(sliceFile)
                continue
            }

            // Check if any generated Java file is missing or outdated
            for (generatedFilePath in generatedFiles) {
                val generatedFile = File(generatedFilePath) // Ensure correct absolute path

                // Recompile if the generated file is missing
                if (!generatedFile.exists()) {
                    logger.lifecycle("Missing generated file: $generatedFile")
                    changedFiles.add(sliceFile)
                    break
                }

                // Recompile if the generated file is older than the Slice file
                if (generatedFile.lastModified() < sliceLastModified) {
                    logger.lifecycle("Outdated generated file: $generatedFile")
                    changedFiles.add(sliceFile)
                    break
                }
            }

            // If the file is not in `dependencies`, assume it's new and must be compiled
            if (sliceFile !in dependencies) {
                logger.lifecycle("New Slice file detected: $sliceFile (No previous dependencies found)")
                changedFiles.add(sliceFile)
                continue
            }

            // Retrieve dependencies (could be empty)
            val sliceDependencies = dependencies[sliceFile].orEmpty()

            // Check if any dependency is newer than the first generated file
            val changedDependency = sliceDependencies.find { it.exists() && it.lastModified() > lastCompiledTimestamp }
            if (changedDependency != null) {
                logger.lifecycle("Dependency changed for Slice file: $sliceFile (Updated dependency: $changedDependency, last compiled at $lastCompiledTimestamp)")
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

    private fun mergeGeneratedFiles(
        previousGeneratedFiles: Map<File, List<String>>,
        newGeneratedFiles: Map<File, List<String>>,
        sliceFiles: List<File>,
    ): Map<File, List<String>> {
        val mergedFiles = previousGeneratedFiles.toMutableMap()

        // For each slice file in the new generated files, add the new files to the merged map. This ensures that
        // we only keep the files that are still being generated.
        newGeneratedFiles.forEach { (sliceFile, files) ->
            mergedFiles[sliceFile] = files
        }

        // Keep only the files that are still in the sliceFiles list. This ensures that we don't keep generated files
        // for deleted slice files.
        mergedFiles.keys.retainAll(sliceFiles)

        return mergedFiles
    }

    /**
     * Saves the generated files map to `generated.xml`, or deletes the file if empty.
     *
     * @param generatedFilesMap The map of Slice source files to their generated Java files.
     */
    private fun saveGeneratedFiles(generatedFilesMap: Map<File, List<String>>) {
        if (generatedFilesMap.isEmpty()) {
            // Remove `generated.xml` if there's nothing to save
            if (generatedXmlFile.exists()) {
                generatedXmlFile.delete()
            }
            return
        }

        val documentBuilderFactory = DocumentBuilderFactory.newInstance()
        val documentBuilder: DocumentBuilder = documentBuilderFactory.newDocumentBuilder()
        val doc = documentBuilder.newDocument()

        val rootElement = doc.createElement("generated")
        doc.appendChild(rootElement)

        for ((sliceFile, files) in generatedFilesMap) {
            val sourceElement = doc.createElement("source")
            sourceElement.setAttribute("name", sliceFile.absolutePath)
            rootElement.appendChild(sourceElement)

            for (file in files) {
                val fileElement = doc.createElement("file")
                fileElement.setAttribute("name", file)
                sourceElement.appendChild(fileElement)
            }
        }

        // Serialize XML properly
        val transformerFactory = TransformerFactory.newInstance()
        val transformer = transformerFactory.newTransformer()
        transformer.setOutputProperty(OutputKeys.INDENT, "yes") // Pretty-printing
        transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2")

        val source = DOMSource(doc)
        val result = StreamResult(generatedXmlFile)
        transformer.transform(source, result)
    }
}
