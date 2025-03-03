// Copyright (c) ZeroC, Inc.

import org.gradle.api.Action
import org.gradle.api.NamedDomainObjectContainer
import org.gradle.api.Project

abstract class SliceExtension(project: Project) {
    val sourceSets: NamedDomainObjectContainer<SliceSourceSet> =
        project.container(SliceSourceSet::class.java) { name -> SliceSourceSet(name, project) }

    fun sourceSets(action: Action<NamedDomainObjectContainer<SliceSourceSet>>) {
        action.execute(sourceSets)
    }
}
