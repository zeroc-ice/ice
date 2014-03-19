// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.decorator;

import java.util.List;

import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.jface.viewers.ILabelDecorator;
import org.eclipse.jface.viewers.ILabelProviderListener;
import org.eclipse.swt.graphics.Image;

import com.zeroc.slice2javaplugin.internal.Configuration;

public class SliceDecorator implements ILabelDecorator
{

    public void addListener(ILabelProviderListener arg0)
    {
    }

    public void dispose()
    {
    }

    public boolean isLabelProperty(Object arg0, String arg1)
    {
        return false;
    }

    public void removeListener(ILabelProviderListener arg0)
    {
    }

    public Image decorateImage(Image arg0, Object arg1)
    {
        return null;
    }

    public String decorateText(String label, Object object)
    {
        IResource resource = (IResource) object;
        if(resource.getType() != IResource.FOLDER)
        {
          // Only folders are decorated.
          return null;
        }
        IProject project = resource.getProject();
        Configuration configuration = new Configuration(project);
        List<String> slices = configuration.getSliceSourceDirs();
        
        boolean decorated = false;
        for(int i = 0; i < slices.size(); ++i)
        {
            IFolder f = project.getFolder(slices.get(i));
            if(f.getLocation().toOSString().equals(resource.getLocation().toOSString()))
            {
                decorated = true;
                break;
            }
        }

        return decorated ? label + " [Slice sources]": label; 
    }

}
