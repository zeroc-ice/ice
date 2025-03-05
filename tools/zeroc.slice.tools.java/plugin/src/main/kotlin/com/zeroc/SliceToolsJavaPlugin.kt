// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.tasks.SourceSet
import org.gradle.api.GradleException
import org.gradle.api.plugins.ExtensionAware

class SliceToolsJavaPlugin : Plugin<Project> {
    override fun apply(project: Project) {

        val isJavaProject = SliceToolsUtil.isJavaProject(project)
        val isAndroidProject = SliceToolsUtil.isAndroidProject(project)
        if (isJavaProject || isAndroidProject) {
            val extension = project.extensions.create("slice", SliceExtension::class.java, project)

            // Aggregator task for all Slice compilation tasks
            val compileSlice = project.tasks.register("compileSlice") {
                it.group = "build"
                it.description = "Runs all Slice compilation tasks."
            }

            if (isJavaProject) {
                SliceToolsJava.configure(project, extension, compileSlice)
            } else {
                SliceToolsAndroid.configure(project, extension, compileSlice)
            }
        } else {
            throw GradleException("The Slice Tools plugin requires a Java or Android project.")
        }
    }
}
