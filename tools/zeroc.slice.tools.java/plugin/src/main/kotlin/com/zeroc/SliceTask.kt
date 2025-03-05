// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.DefaultTask
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.tasks.*
import java.io.File

@CacheableTask
abstract class SliceTask : DefaultTask() {

    /** The Slice source files to compile */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val slice: SourceDirectorySet

    /** Directories to include when searching for Slice files */
    @get:InputFiles
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val includeSliceDirs: ConfigurableFileCollection

    /** Output directory for generated Java files */
    @get:OutputDirectory
    abstract val output: DirectoryProperty

    @TaskAction
    fun compileSlice() {
        val outputDir = output.get().asFile
        outputDir.mkdirs()

        val sliceFiles = slice.sourceDirectories.files.flatMap { dir ->
            dir.listFiles { file -> file.extension == "ice" }?.toList() ?: emptyList()
        }

        if (sliceFiles.isEmpty()) {
            logger.warn("No Slice files found in ${slice.sourceDirectories.files}")
            return
        }

        val includeArgs = includeSliceDirs.files.flatMap { listOf("-I", it.absolutePath) }
        val sliceArgs = sliceFiles.map { it.absolutePath }

        val command = listOf("slice2java") + includeArgs + listOf("--output-dir", outputDir.absolutePath) + sliceArgs

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
