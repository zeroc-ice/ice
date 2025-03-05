// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.model.ObjectFactory
import org.gradle.api.file.ConfigurableFileCollection
import javax.inject.Inject

interface SliceSourceSet {
    val name: String
    val slice: SourceDirectorySet
    val includeSliceDirs: ConfigurableFileCollection
    val output: DirectoryProperty
}

abstract class DefaultSliceSourceSet @Inject constructor(
    override val name: String,
    objects: ObjectFactory
) : SliceSourceSet {
    
    override val slice: SourceDirectorySet = objects.sourceDirectorySet("slice", "$name Slice Source")
    override val includeSliceDirs: ConfigurableFileCollection = objects.fileCollection()
    override val output: DirectoryProperty = objects.directoryProperty()
}
