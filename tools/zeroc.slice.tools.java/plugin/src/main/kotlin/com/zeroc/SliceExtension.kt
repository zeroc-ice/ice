// Copyright (c) ZeroC, Inc.

package com.zeroc

import org.gradle.api.Action
import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project

abstract class SliceExtension(project: Project) {
    val sourceSets = project.objects.domainObjectContainer(SliceSourceSet::class.java) { name ->
        project.objects.newInstance(DefaultSliceSourceSet::class.java, name, project.objects)
    }
}
