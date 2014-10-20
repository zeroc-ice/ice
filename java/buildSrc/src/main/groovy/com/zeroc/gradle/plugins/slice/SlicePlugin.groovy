// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Plugin
import org.gradle.api.Project

class SlicePlugin implements Plugin<Project> {
	void apply(Project project) {
		project.task('generateSliceTask', type: SliceTask) {
			group = "Slice"
		}

        // Create and install the extension object.
        project.extensions.create("slice", SliceExtension, project.container(Java))

        project.extensions.slice.extensions.create("freezej", FreezejExtension,
        	 project.container(Dict), project.container(Index))

        project.slice.output = project.file("${project.buildDir}/generated-src")

        project.getTasks().getByName("compileJava").dependsOn('generateSliceTask');
        project.sourceSets.main.java.srcDir project.slice.output
	}
}
