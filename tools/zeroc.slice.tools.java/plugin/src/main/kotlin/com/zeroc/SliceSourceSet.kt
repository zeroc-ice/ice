// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.model.ObjectFactory
import org.gradle.api.provider.ListProperty
import javax.inject.Inject

/**
 * A set of Slice source files, and associated configuration for compiling them with the Slice to Java compiler.
 */
interface SliceSourceSet : SourceDirectorySet {
    /**
     * The include search path for the Slice source set. Passed to the Slice compiler as the -I option when compiling
     * the Slice files in this source set.
     */
    val includeSearchPath: ConfigurableFileCollection

    /**
     * Additional compiler arguments for the Slice source set. Passed to the Slice compiler as additional arguments
     * when compiling the Slice files in this source set.
     */
    val compilerArgs: ListProperty<String>
}

abstract class DefaultSliceSourceSet @Inject constructor(
    name: String,
    objects: ObjectFactory,
) : SliceSourceSet, SourceDirectorySet by objects.sourceDirectorySet(name, "$name Slice Source") {
    override val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()
    override val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)
}
