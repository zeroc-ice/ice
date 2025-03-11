// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

import org.gradle.api.Project
import java.net.JarURLConnection
import java.util.jar.JarFile
import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption

/**
 * Extracts the Slice compiler (slice2java) and Slice files from the plugin resources to the Gradle user home directory.
 */
object SliceToolsResourceExtractor {
    private const val pluginName = "slice-tools"

    /**
     * Extracts the Slice compiler (slice2java) from the plugin resources to the Gradle user home directory.
     *
     * @param project The Gradle project
     * @return The path to the directory containing the Slice compiler, or null if the extraction failed or the
     * compiler is not present in the resources.
     */
    fun extractSliceCompiler(project: Project): String? {
        val pluginVersion = getPluginVersion(project)

        val os = detectOS()
        val arch = detectArch()
        val toolName = if (os == "windows") "slice2java.exe" else "slice2java"
        val resourcePath = "/resources/$os-$arch/$toolName"

        val gradleUserDir = File(project.gradle.gradleUserHomeDir, "native/$pluginName-$pluginVersion/bin")
        val slice2JavaFile = File(gradleUserDir, toolName)

        if (!slice2JavaFile.exists()) {
            val resourceStream = this::class.java.getResourceAsStream(resourcePath)
            if (resourceStream != null) {
                project.logger.lifecycle("Extracting $toolName to ${gradleUserDir.absolutePath}")
                gradleUserDir.mkdirs()
                Files.copy(resourceStream, slice2JavaFile.toPath(), StandardCopyOption.REPLACE_EXISTING)
                slice2JavaFile.setExecutable(true)
            } else {
                return null
            }
        }

        return gradleUserDir.absolutePath
    }

    /**
     * Extracts the Slice files from the plugin resources to the Gradle user home directory.
     *
     * @param project The Gradle project
     * @return The path to the directory containing the Slice files, or null if the extraction failed or the
     * Slice files are not present in the resources.
     */
    fun extractSliceFiles(project: Project): String? {
        val resourcePath = "/resources/slice/"
        val pluginVersion = getPluginVersion(project)
        val targetDir = File(project.gradle.gradleUserHomeDir, "native/$pluginName-$pluginVersion/slice")

        targetDir.mkdirs() // Ensure the target directory exists

        val resourceURL = this::class.java.getResource("$resourcePath")
        if (resourceURL == null) {
            return null
        }

        val jarConnection = resourceURL.openConnection() as? JarURLConnection
        val jarFile = jarConnection?.jarFile
            ?: throw IllegalStateException("Failed to open jar file: $resourceURL")

        val resourcePaths = jarFile.entries().asSequence()
            .map { it.name }
            .filter { it.endsWith(".ice") }
            .toList()

        for (resource in resourcePaths) {
            project.logger.lifecycle("Extracting $resource to ${targetDir.absolutePath}")
            val relativePath = resource.removePrefix(resourcePath)
            val outputFile = File(targetDir, relativePath)

            outputFile.parentFile.mkdirs() // Create parent directories if needed

            this::class.java.getResourceAsStream("/$resource")?.use { input ->
                outputFile.outputStream().use { output -> input.copyTo(output) }
            }
        }
        return targetDir.absolutePath
    }

    private fun getPluginVersion(project: Project): String {
        val pkg = this::class.java.`package`
        val implementationVersion = pkg?.implementationVersion
        val projectSliceToolsVersion = project.findProperty("sliceToolsVersion") as? String
        return implementationVersion
            ?: projectSliceToolsVersion
            ?: throw IllegalStateException("Failed to determine plugin version: implementationVersion is missing")
    }

    private fun detectOS(): String {
        val os = System.getProperty("os.name").lowercase()
        return when {
            os.contains("win") -> "windows"
            os.contains("mac") -> "macos"
            os.contains("nix") || os.contains("nux") -> "linux"
            else -> os
        }
    }

    private fun detectArch(): String {
        val arch = System.getProperty("os.arch").lowercase()
        return when {
            arch.contains("amd64") || arch.contains("x86_64") -> "x64"
            arch.contains("aarch64") -> "arm64"
            else -> arch
        }
    }
}
