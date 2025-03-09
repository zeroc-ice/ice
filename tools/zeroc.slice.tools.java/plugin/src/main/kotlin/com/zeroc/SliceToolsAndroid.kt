// Copyright (c) ZeroC, Inc.

package com.zeroc

import com.android.build.api.dsl.CommonExtension
import com.android.build.api.variant.AndroidComponentsExtension
import com.android.build.api.variant.Variant
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
        val androidExtension = project.extensions.findByType(CommonExtension::class.java)
            ?: throw GradleException(
                "Android Gradle extension not found. " +
                    "Ensure that either the Android application or library plugin is applied before configuring Slice Tools.",
            )

        val compileSliceTasks = mutableMapOf<String, TaskProvider<SliceTask>>()

        // Create a Slice source set for each Android source set and register it as an extension.
        // A corresponding Slice compile task is registered for each Slice source set.
        androidExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = SliceToolsUtil.createSliceSourceSet(extension, sourceSet.name)

            // Register it as an extension in the Android source set
            (sourceSet as ExtensionAware).extensions.add("slice", sliceSourceSet)

            // Create the compile task for this source set
            val compileTask = SliceToolsUtil.configureSliceTaskForSourceSet(
                project,
                toolsPath,
                includeSearchPath,
                extension,
                sliceSourceSet,
                compileSlice,
            )
            compileSliceTasks[sourceSet.name] = compileTask
        }

        val flavorDimensions = androidExtension.flavorDimensions

        val androidComponents = project.extensions.findByType(AndroidComponentsExtension::class.java)
            ?: throw GradleException("AndroidComponentsExtension is missing. Ensure the Android Gradle Plugin is applied.")

        // Iterate over all variants and link the generated source directories from Slice tasks
        // to the variant's Java sources based on the included source sets.
        androidComponents.onVariants { variant ->
            val sourceSetNames = computeSourceSetsForVariant(variant, flavorDimensions)
            for (sourceSetName in sourceSetNames) {
                val compileSliceTask = compileSliceTasks[sourceSetName]
                    ?: throw GradleException("Slice task for source set $sourceSetName not found in task map.")

                variant.sources.java?.addGeneratedSourceDirectory(
                    compileSliceTask,
                    wiredWith = { it.output },
                )
            }
        }
    }

    /**
     * Computes the source sets included in the given variant based on its flavor dimensions.
     *
     * @param variant The Android variant.
     * @param flavorDimensions The ordered list of flavor dimensions.
     * @return A list of source set names corresponding to the variant.
     */
    private fun computeSourceSetsForVariant(
        variant: Variant,
        flavorDimensions: List<String>,
    ): List<String> {
        val sourceSets = mutableListOf<String>()

        // Always include "main" source set
        sourceSets.add("main")

        // Build type source set (e.g., "debug", "release")
        val buildType = variant.buildType
        if (!buildType.isNullOrEmpty()) {
            sourceSets.add(buildType)
        }

        // Extract flavors and ensure they are ordered by dimension
        val flavorMap = variant.productFlavors.toMap() // Dimension -> Flavor Name
        val flavors = flavorDimensions.mapNotNull { flavorMap[it] } // Retrieve flavors in correct order
        sourceSets.addAll(flavors)

        if (flavors.isNotEmpty()) {
            // Merge multiple flavors correctly: lowercase first, capitalize subsequent
            val mergedFlavors = flavors.first() + flavors.drop(1).joinToString("") { it.replaceFirstChar { c -> c.uppercaseChar() } }
            sourceSets.add(mergedFlavors)

            if (!buildType.isNullOrEmpty()) {
                sourceSets.add(mergedFlavors + buildType.replaceFirstChar { it.uppercaseChar() })
            }
        }

        return sourceSets
    }
}
