package com.zeroc

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project
import org.gradle.api.plugins.ExtensionAware
import org.gradle.api.tasks.TaskProvider
import org.gradle.api.Task
import org.gradle.api.GradleException

object SliceToolsAndroid {
    fun configure(project: Project, extension: SliceExtension, compileSlice: TaskProvider<Task>) {
        val androidExtension = project.extensions.findByType(
            com.android.build.gradle.internal.dsl.BaseAppModuleExtension::class.java
        ) ?: throw GradleException("Android BaseExtension is missing. Ensure the Android plugin is applied before configuring Slice Tools.")

        // Register Slice source sets in both Android source sets & variants
        androidExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = extension.sourceSets.create(sourceSet.name)

            // Register it as an extension in the Android source set
            SliceToolsUtil.addSourceSetExtension(project, sourceSet.name, sourceSet as ExtensionAware, sliceSourceSet)

            // Create a compile task for this Slice source set
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(project, extension, sliceSourceSet, compileSlice)

            // Link the generated output from Slice compilation to the Java sources
            sourceSet.java.srcDir(compileTask.flatMap { it.output })
        }
    }
}
