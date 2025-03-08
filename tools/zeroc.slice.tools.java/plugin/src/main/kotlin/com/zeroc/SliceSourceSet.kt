// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.Project
import org.gradle.api.file.SourceDirectorySet
import org.gradle.api.model.ObjectFactory
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Provider
import org.gradle.api.tasks.TaskProvider
import org.gradle.api.file.FileCollection
import org.gradle.api.tasks.util.PatternFilterable
import javax.inject.Inject
import java.io.File

interface SliceSourceSet : SourceDirectorySet {
    val includeSearchPath: ConfigurableFileCollection
    val compilerArgs: ListProperty<String>
}

abstract class DefaultSliceSourceSet @Inject constructor(
    name: String,
    objects: ObjectFactory
) : SliceSourceSet, SourceDirectorySet by objects.sourceDirectorySet(name, "$name Slice Source")  {
    // Additional properties for Slice-specific configurations
    override val includeSearchPath: ConfigurableFileCollection = objects.fileCollection()
    override val compilerArgs: ListProperty<String> = objects.listProperty(String::class.java)
}
