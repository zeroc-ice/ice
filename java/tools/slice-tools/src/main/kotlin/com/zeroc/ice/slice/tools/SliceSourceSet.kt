// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import javax.inject.Inject

/**
 * A set of Slice source files and associated configuration for compiling them with the Slice-to-Java compiler
 * (`slice2java`).
 *
 * This source set defines the directories containing `.ice` files, additional compiler arguments, and include paths
 * for locating included Slice definitions.
 */
interface SliceSourceSet : SourceDirectorySet {

    /**
     * Additional compiler arguments specific to this Slice source set.
     *
     * These arguments are passed to the Slice compiler when compiling the `.ice` files in this source set.
     */
    val compilerArgs: ListProperty<String>

    /**
     * The include search paths for this Slice source set.
     *
     * These directories are passed to the Slice compiler using the `-I` option.
     */
    val includeSearchPath: ConfigurableFileCollection
}

abstract class DefaultSliceSourceSet @Inject constructor(
    name: String,
    objects: ObjectFactory,
) : SliceSourceSet, SourceDirectorySet by objects.sourceDirectorySet(name, "$name Slice Source") {
    override val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()
    override val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)
}
