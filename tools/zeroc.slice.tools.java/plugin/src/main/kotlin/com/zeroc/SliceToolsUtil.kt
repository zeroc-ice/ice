// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.Project
import org.gradle.api.Task
import org.gradle.api.provider.Provider
import org.gradle.api.tasks.TaskProvider

object SliceToolsUtil {
    /**
     * Checks if the given Gradle project is an Android project.
     *
     * This method determines whether an Android-related Gradle plugin is applied by checking for `com.android.base`,
     * which is the common parent plugin for all Android plugins in AGP 3.0.0+.
     *
     * Since Ice 3.8 targets SDK 34+ (AGP 8.1+), older legacy plugins (such as `android` and `android-library`) are no
     * longer checked.
     *
     * @param project The Gradle project instance.
     * @return `true` if the project applies an Android plugin, otherwise `false`.
     */
    fun isAndroidProject(project: Project): Boolean {
        return project.plugins.hasPlugin("com.android.base")
    }

    /**
     * Checks if the given Gradle project is a Java project.
     *
     * This method detects whether the project applies any Java-related Gradle plugin, ensuring compatibility with both
     * application and library projects.
     *
     * @param project The Gradle project instance.
     * @return `true` if the project applies a Java plugin, otherwise `false`.
     */
    fun isJavaProject(project: Project): Boolean {
        return project.plugins.hasPlugin("java") || project.plugins.hasPlugin("java-library")
    }

    fun isWindows(): Boolean {
        return System.getProperty("os.name").lowercase().contains("win")
    }

    /**
     * Creates a Slice source set with the given name.
     */
    fun createSliceSourceSet(extension: SliceExtension, name: String): SliceSourceSet {
        val sourceSet = extension.sourceSets.create(name)
        sourceSet.srcDir("src/$name/slice")
        sourceSet.include("**/*.ice")
        return sourceSet
    }

    /**
     * Configures a Slice compilation task for the given source set.
     */
    fun configureSliceTaskForSourceSet(
        project: Project,
        toolsPath: Provider<String>,
        includeSearchPath: Provider<List<String>>,
        extension: SliceExtension,
        sliceSourceSet: SliceSourceSet,
        compileSlice: TaskProvider<Task>,
    ): TaskProvider<SliceTask> {
        val taskName = "compileSlice${sliceSourceSet.name.replaceFirstChar { it.uppercaseChar() }}"
        project.logger.lifecycle("Configuring Slice compilation task: $taskName")
        val compileTask = project.tasks.register(taskName, SliceTask::class.java) {
            it.slice.setFrom(sliceSourceSet)
            it.outputBaseName.set(sliceSourceSet.name)

            // Merge include directories from both extension and source set
            it.includeSearchPath.setFrom(includeSearchPath.get() + extension.includeSearchPath + sliceSourceSet.includeSearchPath)

            // Merge compiler arguments from both extension and source set
            it.compilerArgs.set(
                extension.compilerArgs.getOrElse(emptyList()) +
                    sliceSourceSet.compilerArgs.getOrElse(emptyList()),
            )

            // Set Ice configuration
            it.toolsPath.set(toolsPath)
        }
        compileSlice.configure { it.dependsOn(compileTask) }
        return compileTask
    }
}
