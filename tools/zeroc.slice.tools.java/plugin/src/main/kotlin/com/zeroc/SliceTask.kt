// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.DefaultTask
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import org.gradle.api.model.ObjectFactory
import org.gradle.api.tasks.*
import java.io.File
import javax.inject.Inject

@CacheableTask
abstract class SliceTask @Inject constructor(objects: ObjectFactory) : DefaultTask() {

    /** The Slice source files to compile */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val slice: ConfigurableFileCollection

    /** The source set name associated with this task */
    @get:Input
    abstract val sourceSetName: Property<String>

    /** Directories to include when searching for Slice files */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val includeSliceDirs: ConfigurableFileCollection

    /** Additional compiler arguments */
    @get:Input
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /** Path to Ice installation home directory (used to locate Slice files) */
    @get:Input
    abstract val iceHome: Property<String>

    /** Path to Ice tools directory (used to locate the slice2java compiler) */
    @get:Input
    abstract val iceToolsPath: Property<String>

    /** Output directory for generated Java files */
    @get:OutputDirectory
    abstract val output: DirectoryProperty

    init {
        // Set default output directory to `build/generated/source/slice/<sourceSetName>`
        output.convention(
            sourceSetName.flatMap { project.layout.buildDirectory.dir("generated/source/slice/$it") }
        )
    }

    @TaskAction
    fun compileSlice() {
        val outputDir = output.get().asFile
        outputDir.mkdirs()

        val sliceFiles = slice.files.toList()

        if (sliceFiles.isEmpty()) {
            logger.warn("No Slice files found in ${slice.files}")
            return
        }

        val iceHomeDir = iceHome.orNull?.let { File(it, "slice") }

        val includeArgs = includeSliceDirs.files.flatMap { listOf("-I", it.absolutePath) } +
            (iceHomeDir?.takeIf { it.exists() }?.let { listOf("-I", it.absolutePath) } ?: emptyList())

        val sliceArgs = sliceFiles.map { it.absolutePath }

        // Determine the location of `slice2java`
        val slice2javaExe = when {
            SliceToolsUtil.isWindows() -> "slice2java.exe"
            else -> "slice2java"
        }
        val slice2javaPath = iceToolsPath.orNull?.let { "$it/$slice2javaExe" } ?: slice2javaExe

        val command = listOf(slice2javaPath) + includeArgs + listOf("--output-dir", outputDir.absolutePath) + sliceArgs

        logger.lifecycle("Running: ${command.joinToString(" ")}")

        val process = ProcessBuilder(command)
            .directory(project.projectDir)
            .redirectErrorStream(true)
            .start()

        process.inputStream.bufferedReader().use { reader ->
            reader.lines().forEach { logger.lifecycle(it) }
        }

        val exitCode = process.waitFor()
        if (exitCode != 0) {
            throw RuntimeException("slice2java failed with exit code $exitCode")
        }
    }
}
