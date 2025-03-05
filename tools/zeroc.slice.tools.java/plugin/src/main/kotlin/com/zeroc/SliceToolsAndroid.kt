package com.zeroc

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project
import org.gradle.api.plugins.ExtensionAware
import org.gradle.api.tasks.TaskProvider
import org.gradle.api.Task
import org.gradle.api.GradleException

object SliceToolsAndroid {
    fun configure(project: Project, extension: SliceExtension, compileSlice: TaskProvider<Task>) {
        val androidExtension = project.extensions.findByType(com.android.build.gradle.BaseExtension::class.java) ?: return
            ?: throw GradleException("Android extension is missing. Ensure the Android plugin is applied before configuring Slice Tools.")

        androidExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = extension.sourceSets.create(sourceSet.name)
            SliceToolsUtil.addSourceSetExtension(sourceSet.name, sourceSet as ExtensionAware, sliceSourceSet)
            SliceToolsUtil.configureSliceTaskForSourceSet(project, sliceSourceSet, compileSlice)
            sourceSet.java.srcDir(sliceSourceSet.output)
        }
    }
}
