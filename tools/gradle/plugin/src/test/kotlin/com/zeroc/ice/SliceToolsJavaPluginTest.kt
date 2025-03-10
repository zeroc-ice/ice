// Copyright (c) ZeroC, Inc.

package com.zeroc.ice

import org.gradle.testfixtures.ProjectBuilder
import kotlin.test.Test
import kotlin.test.assertNotNull

/**
 * A simple unit test for the 'com.zeroc.ice.slice-tools' plugin.
 */
class SliceToolsJavaPluginTest {
    @Test fun `plugin registers task`() {
        // Create a test project and apply the plugin
        val project = ProjectBuilder.builder().build()
        project.extensions.extraProperties.set("sliceToolsVersion", "3.8.0-alpha.0")
        project.plugins.apply("java")
        project.plugins.apply("com.zeroc.ice.slice-tools")

        // Set toolsPath in the plugin extension
        val extension = project.extensions.getByType(SliceExtension::class.java)
        extension.toolsPath.set("/opt/Ice-3.8.0a/bin")

        // Verify the result
        assertNotNull(project.tasks.findByName("compileSlice"))
    }
}
