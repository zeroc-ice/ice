// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.GradleException
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.provider.Provider

class SliceToolsJavaPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val isJavaProject = SliceToolsUtil.isJavaProject(project)
        val isAndroidProject = SliceToolsUtil.isAndroidProject(project)
        if (isJavaProject || isAndroidProject) {
            val extension = project.extensions.create("slice", SliceExtension::class.java, project)

            // Always attempt to extract slice2java and slice files in apply(), they can be null for source distribution
            // build where we don't include the binaries and slice files in the plugin JAR resources.
            val extractedSlice2Java: String? = SliceToolsResourceExtractor.extractSliceCompiler(project)
            val extractedSliceFiles: String? = SliceToolsResourceExtractor.extractSliceFiles(project)

            // Lazily evaluate toolsPath in tasks
            val toolsPath: Provider<String> = extension.toolsPath.map { it }
                .orElse(
                    project.provider {
                        extractedSlice2Java ?: throw GradleException(
                            "Neither toolsPath is set nor could slice2java be extracted. " +
                                "Ensure that either toolsPath is configured or the plugin resources contain the necessary binaries.",
                        )
                    },
                )

            val includeSearchPath: Provider<List<String>> = extension.toolsPath.map { emptyList<String>() }
                .orElse(
                    project.provider<List<String>> {
                        extractedSliceFiles?.let { listOf(it) } ?: throw GradleException(
                            "Neither toolsPath is set nor could Slice files be extracted. " +
                                "Ensure that either toolsPath is configured or the plugin resources contain the necessary Slice files.",
                        )
                    },
                )

            // Aggregator task for all Slice compilation tasks
            val compileSlice = project.tasks.register("compileSlice") {
                it.group = "build"
                it.description = "Runs all Slice compilation tasks."
            }

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
