// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import javax.inject.Inject

/**
 * Extension for the Slice Tools Gradle plugin.
 *
 * The extension allows configuring the Slice to Java compiler (slice2java) and Slice source sets.
 */
abstract class SliceExtension @Inject constructor(project: Project, objects: ObjectFactory) {

    /** Default compiler arguments applied to all source sets */
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /** Default include directories for Slice files */
    val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()

    /** The path to the directory containing the Slice to java compiler (slice2java)
     *
     * When not set the plugin will attempt to use the slice2java executable and Slice files from the
     * the plugin resources.
     */
    val toolsPath: Property<String> = objects.property(String::class.java)

    /** Slice source sets container */
    val sourceSets: NamedDomainObjectContainer<SliceSourceSet> =
        project.objects.domainObjectContainer(SliceSourceSet::class.java) { name ->
            project.objects.newInstance(DefaultSliceSourceSet::class.java, name, project.objects)
        }
}
