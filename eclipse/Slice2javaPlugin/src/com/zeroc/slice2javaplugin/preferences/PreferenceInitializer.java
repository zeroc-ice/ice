// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
        String os = System.getProperty("os.name");
        if(os.equals("Linux"))
        {
            return "/usr";
        }
        else if(os.startsWith("Windows"))
        {
            File f = new File("C:\\Ice-3.3.0");
            if(!f.exists())
            {
                File f2 = new File("C:\\Ice-3.3.0-VC90");
                if(f2.exists())
                {
                    return f2.toString();
                }
            }
            return f.toString();
        }
        return "/opt/Ice-3.3.0";
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
