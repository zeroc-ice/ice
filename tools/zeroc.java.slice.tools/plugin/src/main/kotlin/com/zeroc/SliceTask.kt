// Copyright (c) ZeroC, Inc.

import org.gradle.api.DefaultTask
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.TaskAction

abstract class SliceTask : DefaultTask() {
    @get:OutputDirectory
    abstract val outputDir: Property<org.gradle.api.file.Directory>

    @TaskAction
    fun generateSliceFiles() {
        println("Generating Slice files into ${outputDir.get().asFile}")
        // Actual logic for generating Slice files
    }
}
