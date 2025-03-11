// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

import org.gradle.api.GradleException
import org.gradle.api.Project
import org.gradle.api.Task
import org.gradle.api.plugins.JavaPluginExtension
import org.gradle.api.provider.Provider
import org.gradle.api.tasks.TaskProvider

/**
 * Configures Slice source sets and tasks for Java projects using the Gradle Java Plugin.
 *
 * This object integrates the Slice compilation workflow into the Java build system by:
 * - Registering a Slice source set for each Java source set.
 * - Creating and associating a `compileSlice` task for each source set.
 * - Adding the generated Slice output directory to the Java source set.
 */
object SliceToolsJava {
    /**
     * Configures the Slice Tools integration for a Java project.
     *
     * This method ensures that Slice files are properly compiled and included in the Java source sets.
     * It does this by:
     * - Registering a Slice source set for each Java source set.
     * - Creating a `compileSlice` task for each source set.
     * - Linking the output directory of the compiled Slice files to the Java source set.
     *
     * @param project The Gradle project.
     * @param toolsPath The provider for the path to the Ice tools directory.
     * @param includeSearchPath The provider for the include search paths.
     * @param extension The Slice plugin extension containing project-wide configuration.
     * @param compileSlice The top-level `compileSlice` task, used as a dependency for all Slice compilation tasks.
     * @throws GradleException if the Java plugin is not applied.
     */
    fun configure(
        project: Project,
        toolsPath: Provider<String>,
        includeSearchPath: Provider<List<String>>,
        extension: SliceExtension,
        compileSlice: TaskProvider<Task>,
    ) {
        val javaExtension = project.extensions.findByType(JavaPluginExtension::class.java)
            ?: throw GradleException("JavaPluginExtension is missing. Ensure the Java plugin is applied before configuring Slice Tools.")

        // Iterate over all Java source sets and register a Slice source set for each one.
        javaExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = SliceToolsUtil.createSliceSourceSet(extension, sourceSet.name)

            // Register the Slice source set as an extension to the Java source set
            sourceSet.extensions.add("slice", sliceSourceSet)

            // Create the compile task for this source set
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(
                project,
                toolsPath,
                includeSearchPath,
                extension,
                sliceSourceSet,
                compileSlice,
            )

            // Link the output directory of the compiled Slice files to the Java source set
            sourceSet.java.srcDir(compileTask.flatMap { it.output })
        }
    }
}
