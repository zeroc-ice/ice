package com.zeroc

import org.gradle.api.Project
import org.gradle.api.plugins.ExtensionAware
import org.gradle.api.plugins.JavaPluginExtension
import org.gradle.api.tasks.TaskProvider
import org.gradle.api.Task
import org.gradle.api.GradleException
import org.gradle.api.provider.Provider

object SliceToolsJava {
    fun configure(
        project: Project,
        toolsPath: Provider<String>,
        includeSearchPath: Provider<List<String>>,
        extension: SliceExtension,
        compileSlice: TaskProvider<Task>) {
        val javaExtension = project.extensions.findByType(JavaPluginExtension::class.java)
            ?: throw GradleException("JavaPluginExtension is missing. Ensure the Java plugin is applied before configuring Slice Tools.")

        javaExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = extension.sourceSets.create(sourceSet.name)
            SliceToolsUtil.addSourceSetExtension(project, sourceSet.name, sourceSet as ExtensionAware, sliceSourceSet)
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(
                project,
                toolsPath,
                includeSearchPath,
                extension,
                sliceSourceSet,
                compileSlice)
            // Correctly reference the output directory of the Slice compilation task
            sourceSet.java.srcDir(compileTask.flatMap { it.output })
        }
    }
}
