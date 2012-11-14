package com.zeroc.slice2javaplugin.preferences;

import org.eclipse.osgi.util.NLS;

public class Messages extends NLS
{
    private static final String BUNDLE_NAME = "com.zeroc.slice2javaplugin.preferences.messages"; //$NON-NLS-1$
    public static String IceStringVersion;
    static
    {
        // initialize resource bundle
        NLS.initializeMessages(BUNDLE_NAME, Messages.class);
    }

    private Messages()
    {
    }
}
