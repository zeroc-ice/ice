package com.zeroc

import com.android.build.gradle.LibraryExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import org.gradle.api.GradleException
import org.gradle.api.Project
import org.gradle.api.Task
import org.gradle.api.plugins.ExtensionAware
import org.gradle.api.provider.Provider
import org.gradle.api.tasks.TaskProvider

object SliceToolsAndroid {
    fun configure(
        project: Project,
        toolsPath: Provider<String>,
        includeSearchPath: Provider<List<String>>,
        extension: SliceExtension,
        compileSlice: TaskProvider<Task>,
    ) {
        val androidExtension = project.extensions.findByType(BaseAppModuleExtension::class.java)
            ?: project.extensions.findByType(LibraryExtension::class.java)
            ?: throw GradleException(
                "Android extension is missing. Ensure either the Android application or library plugin is applied.\n" +
                    "Ensure the Android plugin is applied before configuring Slice Tools.",
            )

        // Register Slice source sets in both Android source sets & variants
        androidExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = extension.sourceSets.create(sourceSet.name)

            // Register it as an extension in the Android source set
            SliceToolsUtil.addSourceSetExtension(project, sourceSet.name, sourceSet as ExtensionAware, sliceSourceSet)

            // Create a compile task for this Slice source set
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(
                project,
                toolsPath,
                includeSearchPath,
                extension,
                sliceSourceSet,
                compileSlice,
            )

            // Link the generated output from Slice compilation to the Java sources
            sourceSet.java.srcDir(compileTask.flatMap { it.output })
        }
    }
}
