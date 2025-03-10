// Copyright (c) ZeroC, Inc.

package com.zeroc.ice

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import javax.inject.Inject

/**
 * Gradle extension for configuring the Slice Tools plugin.
 *
 * This extension allows configuring the Slice-to-Java compiler (`slice2java`),
 * defining Slice source sets, and setting default compilation options.
 */
abstract class SliceExtension @Inject constructor(project: Project, objects: ObjectFactory) {
    /**
     * Default compiler arguments applied to all Slice source sets.
     * These arguments are passed to `slice2java` during compilation.
     */
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /**
     * Default directories for the Slice include search path.
     *
     * These directories are passed to the Slice compiler using the `-I` option
     * when compiling Slice files. Applied to all source sets.
     */
    val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()

    /**
     * A container for managing Slice source sets.
     *
     * This allows defining multiple named Slice source sets within the project.
     */
    val sourceSets: NamedDomainObjectContainer<SliceSourceSet> =
        project.objects.domainObjectContainer(SliceSourceSet::class.java) { name ->
            project.objects.newInstance(DefaultSliceSourceSet::class.java, name, project.objects)
        }

    /**
     * The path to the directory containing the Slice-to-Java compiler (`slice2java`).
     *
     * If not set, the plugin will attempt to locate `slice2java` and Slice files
     * from the plugin's bundled resources.
     */
    val toolsPath: Property<String> = objects.property(String::class.java)
}
