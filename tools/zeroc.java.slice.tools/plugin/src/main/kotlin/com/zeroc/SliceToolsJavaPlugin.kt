// Copyright (c) ZeroC, Inc.

import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.tasks.TaskProvider

class SliceToolsJavaPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val extension = project.extensions.create("slice", SliceExtension::class.java, project)

        // Aggregator task for all Slice compilation tasks
        val compileSlice = project.tasks.register("compileSlice") {
            it.group = "build"
            it.description = "Runs all Slice compilation tasks."
        }

        project.afterEvaluate {
            extension.sourceSets.all { sourceSet ->
                val taskName = "compileSlice${sourceSet.name.capitalize()}"
                val compileTask = project.tasks.register(taskName, SliceTask::class.java) {
                    it.sliceFiles.setFrom(sourceSet.sliceFiles)
                    it.args.set(sourceSet.args)
                    it.outputDir.set(sourceSet.outputDir)
                }

                compileSlice.configure { it.dependsOn(compileTask) }
                project.tasks.named("compileJava").configure { it.dependsOn(compileSlice) }

                project.extensions.findByType(org.gradle.api.plugins.JavaPluginExtension::class.java)?.apply {
                    sourceSets.getByName("main").java.srcDir(compileTask.map { it.outputDir })
                }
            }
        }
    }
}
