// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.internal;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jdt.core.ClasspathContainerInitializer;
import org.eclipse.jdt.core.IAccessRule;
import org.eclipse.jdt.core.IClasspathAttribute;
import org.eclipse.jdt.core.IClasspathContainer;
import org.eclipse.jdt.core.IClasspathEntry;
import org.eclipse.jdt.core.IJavaProject;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.JavaModelException;

public class IceClasspathContainerIntializer extends ClasspathContainerInitializer
{
    private final static String CONTAINER_ID = "com.zeroc.Slice2JavaPlugin.ICE_FRAMEWORK";

    @Override
    public void initialize(IPath containerPath, IJavaProject project)
        throws CoreException
    {
        if(containerPath.toString().equals(CONTAINER_ID))
        {
            Configuration c = new Configuration(project.getProject());
            configure(c, project, containerPath);
        }
    }

    public static IClasspathEntry getContainerEntry()
    {
        return JavaCore.newContainerEntry(new Path(CONTAINER_ID));
    }

    public static void reinitialize(IProject project, Configuration c)
        throws CoreException
    {
        IJavaProject javaProject = JavaCore.create(project);
        IPath containerPath = new Path(CONTAINER_ID);

        configure(c, javaProject, containerPath);
    }

    private static void configure(Configuration c, IJavaProject javaProject, IPath containerPath)
        throws JavaModelException
    {
        if(c.getAddJars())
        {
            Path dir = new Path(Configuration.getJarDir());
            List<IClasspathEntry> entries = new ArrayList<IClasspathEntry>();
            for(String jar : c.getJars())
            {
                IPath path = dir.append(new Path(jar));
                IClasspathEntry classpathEntry = JavaCore.newLibraryEntry(path, null, null, new IAccessRule[0], new IClasspathAttribute[0], false);
                entries.add(classpathEntry);
            }

            IClasspathContainer container = new IceClasspathContainer(entries.toArray(new IClasspathEntry[0]), containerPath);
            JavaCore.setClasspathContainer(containerPath, new IJavaProject[] { javaProject },
                    new IClasspathContainer[] { container }, new NullProgressMonitor());
        }
    }
    
    public static void updateProjects(String value, List<IJavaProject> projects)
    {
        for(IJavaProject p : projects)
        {
            IPath containerPath = new Path(CONTAINER_ID);
            Configuration c = new Configuration(p.getProject());
            try
            {
                configure(c, p, containerPath);
            }
            catch(JavaModelException e)
            {
                e.printStackTrace();
            }
        }
    }
}
