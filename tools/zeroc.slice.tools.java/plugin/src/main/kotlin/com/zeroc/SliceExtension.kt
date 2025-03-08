// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property
import javax.inject.Inject

abstract class SliceExtension @Inject constructor(project: Project, objects: ObjectFactory) {

    /** Global compiler arguments applied to all source sets */
    val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)

    /** Global include directories for Slice files */
    val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()

    /** Path to Ice tools directory (used to locate the slice2java compiler) */
    val toolsPath: Property<String> = objects.property(String::class.java)

    /** Slice source sets container */
    val sourceSets: NamedDomainObjectContainer<SliceSourceSet> =
        project.objects.domainObjectContainer(SliceSourceSet::class.java) { name ->
            project.objects.newInstance(DefaultSliceSourceSet::class.java, name, project.objects)
        }
}
