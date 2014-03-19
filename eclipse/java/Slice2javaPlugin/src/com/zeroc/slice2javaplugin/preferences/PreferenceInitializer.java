// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.preferences;

import java.io.File;

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.jface.preference.IPreferenceStore;

import com.zeroc.slice2javaplugin.Activator;

/**
 * Class used to initialize default preference values.
 */
public class PreferenceInitializer extends AbstractPreferenceInitializer
{
    private String getDefaultHome()
    {
        String os = System.getProperty("os.name"); //$NON-NLS-1$
        if(os.startsWith("Windows")) //$NON-NLS-1$
        {
            File f = new File("C:\\Program Files\\ZeroC\\Ice-" + Messages.IceStringVersion);
            if(!f.exists())
            {
                File f2 = new File("C:\\Program Files (x86)\\ZeroC\\Ice-" + Messages.IceStringVersion);
                if(f2.exists())
                {
                    return f2.toString();
                }
            }
            return f.toString();
        }
        if(os.equals("Linux")) //$NON-NLS-1$
        {
            File f = new File("/usr/bin/slice2java"); //$NON-NLS-1$
            if(f.exists())
            {
                return "/usr"; //$NON-NLS-1$
            }
        }
        return "/opt/Ice-" + Messages.IceStringVersion; //$NON-NLS-1$
    }

    /*
     * (non-Javadoc)
     * 
     * @seeorg.eclipse.core.runtime.preferences.AbstractPreferenceInitializer#
     * initializeDefaultPreferences()
     */
    public void initializeDefaultPreferences()
    {
        IPreferenceStore store = Activator.getDefault().getPreferenceStore();
        
        store.setDefault(PluginPreferencePage.SDK_PATH, getDefaultHome() );
    }

}
