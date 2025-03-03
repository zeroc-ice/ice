// Copyright (c) ZeroC, Inc.

import org.gradle.api.Named
import org.gradle.api.Project
import org.gradle.api.file.ConfigurableFileCollection
import org.gradle.api.provider.ListProperty
import org.gradle.api.provider.Property

abstract class SliceSourceSet(val name: String, project: Project) : Named {
    override fun getName(): String = name

    val sliceFiles: ConfigurableFileCollection = project.objects.fileCollection()
    val args: ListProperty<String> = project.objects.listProperty(String::class.java)
    val outputDir: Property<String> = project.objects.property(String::class.java)

    init {
        outputDir.convention(project.layout.buildDirectory.dir("generated/slice/$name").map { it.asFile.absolutePath })
    }
}
