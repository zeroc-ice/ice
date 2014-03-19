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

import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jdt.core.ClasspathVariableInitializer;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.JavaModelException;

public class IceClasspathVariableInitializer extends ClasspathVariableInitializer
{
    private final static String VARIABLE_NAME = "ICE_JAR_HOME";

    @Override
    public void initialize(String variable)
    {
        if(variable.equals(VARIABLE_NAME))
        {
            update();
        }
    }

    public static void update()
    {
        IPath path = new Path(Configuration.getJarDir());
        try
        {
            JavaCore.setClasspathVariable(VARIABLE_NAME, path, null);
        }
        catch(JavaModelException e)
        {
            e.printStackTrace();
        }
    }
}