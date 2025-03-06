// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.model.ObjectFactory
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.provider.ListProperty
import javax.inject.Inject

interface SliceSourceSet {
    val name: String
    val slice: SourceDirectorySet
    val includeSliceDirs: ConfigurableFileCollection
    val output: ConfigurableFileCollection
    val compilerArgs: ListProperty<String>
}

abstract class DefaultSliceSourceSet @Inject constructor(
    override val name: String,
    objects: ObjectFactory
) : SliceSourceSet {
    
    override val slice: SourceDirectorySet = objects.sourceDirectorySet("slice", "$name Slice Source")
    override val includeSliceDirs: ConfigurableFileCollection = objects.fileCollection()
    override val output: ConfigurableFileCollection = objects.fileCollection()
    override val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)
}
