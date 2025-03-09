// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.GradleException
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.provider.Provider

/**
 * Gradle plugin for integrating Slice compilation into Java and Android projects.
 *
 * This plugin automatically:
 * - Extracts the `slice2java` compiler and necessary Slice files if they are not explicitly configured.
 * - Configures Slice source sets and compilation tasks for Java and Android projects.
 * - Ensures that `slice2java` is correctly invoked as part of the build process.
 *
 * **Project Compatibility:**
 * - This plugin supports both **Java** and **Android** projects.
 * - It will fail if applied to a project that is neither.
 */
class SliceToolsJavaPlugin : Plugin<Project> {
    /**
     * Applies the Slice Tools plugin to the given Gradle project.
     *
     * - Determines whether the project is a Java or Android project.
     * - Creates and configures the `slice` extension.
     * - Extracts `slice2java` and Slice files into Gradle user directory.
     * - Registers an aggregator `compileSlice` task.
     * - Configures Slice compilation for either Java or Android projects.
     *
     * @param project The Gradle project to which the plugin is applied.
     * @throws GradleException if the project is neither a Java nor an Android project.
     */
    override fun apply(project: Project) {
        val isJavaProject = SliceToolsUtil.isJavaProject(project)
        val isAndroidProject = SliceToolsUtil.isAndroidProject(project)
        if (isJavaProject || isAndroidProject) {
            // Create and register the Slice extension
            val extension = project.extensions.create("slice", SliceExtension::class.java, project)

            // Always attempt to extract `slice2java` and Slice files when applying the plugin.
            val extractedSlice2Java: String? = SliceToolsResourceExtractor.extractSliceCompiler(project)
            val extractedSliceFiles: String? = SliceToolsResourceExtractor.extractSliceFiles(project)

            // Lazily resolve `toolsPath`—first checking the extension, then falling back to extracted `slice2java`.
            val toolsPath: Provider<String> = extension.toolsPath.map { it }
                .orElse(
                    project.provider {
                        extractedSlice2Java ?: throw GradleException(
                            "Neither toolsPath is set nor could slice2java be extracted. " +
                                "Ensure that either toolsPath is configured or the plugin resources contain the necessary binaries.",
                        )
                    },
                )

            // Lazily resolve `includeSearchPath`—first checking the extension, then falling back to extracted Slice files.
            val includeSearchPath: Provider<List<String>> = extension.toolsPath.map { emptyList<String>() }
                .orElse(
                    project.provider<List<String>> {
                        extractedSliceFiles?.let { listOf(it) } ?: throw GradleException(
                            "Neither toolsPath is set nor could Slice files be extracted. " +
                                "Ensure that either toolsPath is configured or the plugin resources contain the necessary Slice files.",
                        )
                    },
                )

            // Aggregator task for all Slice compilation tasks.
            val compileSlice = project.tasks.register("compileSlice") {
                it.group = "build"
                it.description = "Runs all Slice compilation tasks."
            }

            // Configure Slice tools based on project type
            if (isJavaProject) {
                SliceToolsJava.configure(project, toolsPath, includeSearchPath, extension, compileSlice)
            } else {
                SliceToolsAndroid.configure(project, toolsPath, includeSearchPath, extension, compileSlice)
            }
        } else {
            throw GradleException("The Slice Tools plugin requires a Java or Android project.")
        }
    }
}
