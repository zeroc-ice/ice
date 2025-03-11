// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

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

    /**
     * Determines if the current operating system is Windows.
     *
     * @return `true` if the operating system is Windows, otherwise `false`.
     */
    fun isWindows(): Boolean {
        return System.getProperty("os.name").lowercase().contains("win")
    }

    /**
     * Creates and configures a Slice source set with the specified name.
     *
     * This method:
     * - Registers a new Slice source set in the given `SliceExtension`.
     * - Sets the default source directory to `src/<name>/slice`.
     * - Includes all `.ice` files within the source directory.
     *
     * @param extension The Slice extension where the source set will be registered.
     * @param name The name of the Slice source set.
     * @return The newly created and configured `SliceSourceSet`.
     */
    fun createSliceSourceSet(extension: SliceExtension, name: String): SliceSourceSet {
        val sourceSet = extension.sourceSets.create(name)
        sourceSet.srcDir("src/$name/slice")
        sourceSet.include("**/*.ice")
        return sourceSet
    }

    /**
     * Creates and configures a Slice compilation task for the specified source set.
     *
     * This method:
     * - Registers a new `SliceTask` for the given `SliceSourceSet`.
     * - Configures the task with the appropriate input files, include paths, and compiler arguments.
     * - Ensures the generated task is executed as a dependency of the top-level `compileSlice` task.
     *
     * @param project The Gradle project where the task is registered.
     * @param toolsPath The provider for the Ice tools directory path.
     * @param includeSearchPath The provider for the include search paths.
     * @param extension The Slice plugin extension containing project-wide configuration.
     * @param sliceSourceSet The Slice source set for which the compilation task is created.
     * @param compileSlice The top-level `compileSlice` task that aggregates all Slice compilation tasks.
     * @return The registered `SliceTask` for the given source set.
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
        val compileTask = project.tasks.register(taskName, SliceTask::class.java) {
            it.inputFiles.setFrom(sliceSourceSet)
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
