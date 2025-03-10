// Copyright (c) ZeroC, Inc.

package com.zeroc.ice

import org.gradle.testkit.runner.GradleRunner
import org.junit.jupiter.api.io.TempDir
import java.io.File
import kotlin.test.Test

/**
 * A simple functional test for the 'com.zeroc.JavaSliceTools' plugin.
 */
class SliceToolsJavaPluginFunctionalTest {

    @field:TempDir
    lateinit var projectDir: File

    private val buildFile by lazy { projectDir.resolve("build.gradle") }
    private val propsFile by lazy { projectDir.resolve("gradle.properties") }
    private val settingsFile by lazy { projectDir.resolve("settings.gradle") }

    @Test fun `can run task`() {
        // Set up the test build
        settingsFile.writeText("")
        propsFile.writeText("sliceToolsVersion=3.8.0-alpha.0")
        buildFile.writeText(
            """
            plugins {
                id('java')
                id('com.zeroc.ice.slice-tools')
            }

            slice {
                toolsPath = "/opt/Ice-3.8.0a/bin"
            }
            """.trimIndent(),
        )

        // Run the build
        val runner = GradleRunner.create()
        runner.forwardOutput()
        runner.withPluginClasspath()
        runner.withProjectDir(projectDir)
        val result = runner.build()
    }
}
