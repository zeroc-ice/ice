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

/**
 * Configures Slice source sets and tasks for Android projects using the Gradle Android Plugin.
 *
 * This object integrates the Slice compilation workflow into the Android build system by:
 * - Registering a Slice source set for each Android source set.
 * - Creating and associating a `compileSlice` task for each source set.
 * - Wiring the generated source directories to the corresponding Android variant.
 *
 * **Android API Dependency Isolation**:
 * - This is the **only file** in the plugin that directly references Android Gradle Plugin (AGP) APIs.
 * - This ensures that other parts of the plugin remain free of Android dependencies.
 * - Kotlin's lazy class loading prevents AGP-related classes from being loaded when the plugin is used in non-Android
 *   projects.
 */
object SliceToolsAndroid {
    /**
     * Configures the Slice Tools integration for an Android project.
     *
     * @param project The Gradle project.
     * @param toolsPath The provider for the path to the Ice tools directory.
     * @param includeSearchPath The provider for the include search paths.
     * @param extension The Slice plugin extension containing project-wide configuration.
     * @param compileSlice The top-level compileSlice task, used as a dependency for all Slice compilation tasks.
     */
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

        // Register a Slice source set and a corresponding compile task for each Android source set.
        androidExtension.sourceSets.configureEach { sourceSet ->
            val sliceSourceSet = SliceToolsUtil.createSliceSourceSet(extension, sourceSet.name)

            // Register the Slice source set as an extension to the Android source set.
            (sourceSet as ExtensionAware).extensions.add("slice", sliceSourceSet)

            // Create the compile task for this source set and store it in the map.
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
        // to the variant's Java sources based on the source sets included in each variant.
        androidComponents.onVariants { variant ->
            val sourceSetNames = computeSourceSetsForVariant(variant, flavorDimensions)
            for (sourceSetName in sourceSetNames) {
                val compileSliceTask = compileSliceTasks[sourceSetName]
                    ?: throw GradleException("Slice task for source set $sourceSetName not found in task map.")

                // Register the generated source directory for Java compilation
                variant.sources.java?.addGeneratedSourceDirectory(
                    compileSliceTask,
                    wiredWith = { it.output },
                )
            }
        }
    }

    /**
     * Computes the source sets included in the given variant based on its build type, product flavors, and project
     * flavor dimensions.
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

        // Always include the "main" source set
        sourceSets.add("main")

        // Add the build type source set (e.g., "debug", "release") if applicable
        val buildType = variant.buildType
        if (!buildType.isNullOrEmpty()) {
            sourceSets.add(buildType)
        }

        // Retrieve product flavors and ensure they are ordered correctly by dimension
        val flavorMap = variant.productFlavors.toMap() // Dimension -> Flavor Name
        val flavors = flavorDimensions.mapNotNull { flavorMap[it] } // Retrieve flavors in correct order
        sourceSets.addAll(flavors)

        if (flavors.isNotEmpty()) {
            // Merge multiple flavors correctly: keep the first lowercase, capitalize subsequent flavors
            val mergedFlavors = flavors.first() + flavors.drop(1).joinToString("") { it.replaceFirstChar { c -> c.uppercaseChar() } }
            sourceSets.add(mergedFlavors)

            // Add a merged flavor + build type source set if a build type exists
            if (!buildType.isNullOrEmpty()) {
                sourceSets.add(mergedFlavors + buildType.replaceFirstChar { it.uppercaseChar() })
            }
        }

        return sourceSets
    }
}
