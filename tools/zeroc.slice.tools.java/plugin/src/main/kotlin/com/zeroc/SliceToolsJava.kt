// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.GradleException
import org.gradle.api.Project
import org.gradle.api.Task
import org.gradle.api.plugins.JavaPluginExtension
import org.gradle.api.provider.Provider
import org.gradle.api.tasks.TaskProvider

object SliceToolsJava {
    fun configure(
        project: Project,
        toolsPath: Provider<String>,
        includeSearchPath: Provider<List<String>>,
        extension: SliceExtension,
        compileSlice: TaskProvider<Task>,
    ) {
        val javaExtension = project.extensions.findByType(JavaPluginExtension::class.java)
            ?: throw GradleException("JavaPluginExtension is missing. Ensure the Java plugin is applied before configuring Slice Tools.")

        javaExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = SliceToolsUtil.createSliceSourceSet(extension, sourceSet.name)
            sourceSet.extensions.add("slice", sliceSourceSet)
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(
                project,
                toolsPath,
                includeSearchPath,
                extension,
                sliceSourceSet,
                compileSlice,
            )
            // Correctly reference the output directory of the Slice compilation task
            sourceSet.java.srcDir(compileTask.flatMap { it.output })
        }
    }
}
