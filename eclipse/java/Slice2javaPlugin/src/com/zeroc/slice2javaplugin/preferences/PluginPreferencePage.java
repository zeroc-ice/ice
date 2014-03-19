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

import org.eclipse.jface.preference.*;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.eclipse.ui.IWorkbench;

import com.zeroc.slice2javaplugin.Activator;
import com.zeroc.slice2javaplugin.internal.Configuration;

/**
 * This class represents a preference page that is contributed to the
 * Preferences dialog. By subclassing <samp>FieldEditorPreferencePage</samp>, we
 * can use the field support built into JFace that allows us to create a page
 * that is small and knows how to save, restore and apply itself.
 * <p>
 * This page is used to modify preferences only. They are stored in the
 * preference store that belongs to the main plug-in class. That way,
 * preferences can be accessed directly via the preference store.
 */

public class PluginPreferencePage extends FieldEditorPreferencePage implements IWorkbenchPreferencePage
{
    public static final String SDK_PATH = "pathPreference";
    
    public PluginPreferencePage()
    {
        super(GRID);
        setPreferenceStore(Activator.getDefault().getPreferenceStore());
        setDescription("Slice2Java Preferences");
    }

    /**
     * Creates the field editors. Field editors are abstractions of the common
     * GUI blocks needed to manipulate various types of preferences. Each field
     * editor knows how to save and restore itself.
     */
    public void createFieldEditors()
    {
        addField(new SdkDirectoryFieldEditor(SDK_PATH, "&SDK Location:", getFieldEditorParent()));
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * org.eclipse.ui.IWorkbenchPreferencePage#init(org.eclipse.ui.IWorkbench)
     */
    public void init(IWorkbench workbench)
    {
    }

    private static class SdkDirectoryFieldEditor extends DirectoryFieldEditor
    {

        public SdkDirectoryFieldEditor(String name, String labelText, Composite parent)
        {
            super(name, labelText, parent);
            setEmptyStringAllowed(false);
        }

        /**
         * Method declared on StringFieldEditor and overridden in
         * DirectoryFieldEditor. Checks whether the text input field contains a
         * valid directory.
         * 
         * @return True if the apply/ok button should be enabled in the pref
         *         panel
         */
        @Override
        protected boolean doCheckState()
        {
            String dir = getTextControl().getText();
            dir = dir.trim();
            if(!Configuration.verifyIceHome(dir))
            {
                setErrorMessage("Invalid SDK Location");
                return false;
            }
            clearMessage();
            return true;
        }

        @Override
        public Text getTextControl(Composite parent)
        {
            setValidateStrategy(VALIDATE_ON_KEY_STROKE);
            return super.getTextControl(parent);
        }
    }
}
