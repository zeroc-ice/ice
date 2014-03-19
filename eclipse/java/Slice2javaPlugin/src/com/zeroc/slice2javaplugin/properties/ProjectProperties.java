// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This plug-in is provided to you under the terms and conditions
// of the Eclipse Public License Version 1.0 ("EPL"). A copy of
// the EPL is available at http://www.eclipse.org/legal/epl-v10.html.
//
// **********************************************************************

package com.zeroc.slice2javaplugin.properties;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.TabFolder;
import org.eclipse.swt.widgets.TabItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import com.zeroc.slice2javaplugin.Activator;
import com.zeroc.slice2javaplugin.builder.Slice2JavaBuilder;
import com.zeroc.slice2javaplugin.internal.Configuration;

public class ProjectProperties extends PropertyPage
{
    public ProjectProperties()
    {
        super(true);
        setTitle("Slice2Java Settings");
        noDefaultAndApplyButton();
    }

    public void performApply()
    {
        super.performApply();
    }

    public boolean performOk()
    {
        final IProject project = getProject();

        try
        {
            _config.setGeneratedDir(_generatedDir.getText());
            _config.setSliceSourceDirs(Arrays.asList(_sourceDirectories.getItems()));
            _config.setIncludes(Arrays.asList(_includes.getItems()));
            _config.setDefines(Configuration.toList(_defines.getText()));
            _config.setMeta(Configuration.toList(_meta.getText()));
            _config.setStream(_stream.getSelection());
            _config.setTie(_tie.getSelection());
            _config.setIce(_ice.getSelection());
            _config.setUnderscore(_underscore.getSelection());
            _config.setConsole(_console.getSelection());
            _config.setExtraArguments(_extraArguments.getText());
            if(_config.getAddJars())
            {
                java.util.List<String> jars = new ArrayList<String>();
                jars.add("Ice.jar");
                if(_freezeJar.getSelection())
                {
                    jars.add("Freeze.jar");
                }
                if(_glacier2Jar.getSelection())
                {
                    jars.add("Glacier2.jar");
                }
                if(_iceBoxJar.getSelection())
                {
                    jars.add("IceBox.jar");
                }
                if(_iceStormJar.getSelection())
                {
                    jars.add("IceStorm.jar");
                }
                if(_iceGridJar.getSelection())
                {
                    jars.add("IceGrid.jar");
                }
                if(_icePatch2Jar.getSelection())
                {
                    jars.add("IcePatch2.jar");
                }
                _config.setJars(jars);
            }

            if(_config.write())
            {
                // The configuration properties were changed. We need to rebuild
                // the slice files.
                Job job = new Job("Rebuild")
                {
                    protected IStatus run(IProgressMonitor monitor)
                    {
                        try
                        {
                            project.build(IncrementalProjectBuilder.FULL_BUILD, Slice2JavaBuilder.BUILDER_ID, null,
                                    monitor);
                        }
                        catch(CoreException e)
                        {
                            return new Status(Status.ERROR, Activator.PLUGIN_ID, 0, "rebuild failed", e);
                        }
                        return Status.OK_STATUS;
                    }
                };
                job.setPriority(Job.BUILD);
                job.schedule(); // start as soon as possible
            }
        }
        catch(CoreException e)
        {
            return false;
        }
        catch(IOException e)
        {
            ErrorDialog.openError(getShell(), "Error", "Error saving preferences", new Status(Status.ERROR,
                    Activator.PLUGIN_ID, 0, null, e));
            return false;
        }
        return true;
    }

    protected void createPostOptions(Composite composite)
    {
        _jarsGroup = new Group(composite, SWT.NONE);
        _jarsGroup.setText("Add references to the following JAR files:");

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 7;

        _jarsGroup.setLayout(gridLayout);
        _jarsGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        _freezeJar = new Button(_jarsGroup, SWT.CHECK);
        _freezeJar.setText("Freeze");

        _glacier2Jar = new Button(_jarsGroup, SWT.CHECK);
        _glacier2Jar.setText("Glacier2");

        _iceBoxJar = new Button(_jarsGroup, SWT.CHECK);
        _iceBoxJar.setText("IceBox");

        _iceGridJar = new Button(_jarsGroup, SWT.CHECK);
        _iceGridJar.setText("IceGrid");

        _icePatch2Jar = new Button(_jarsGroup, SWT.CHECK);
        _icePatch2Jar.setText("IcePatch2");

        _iceStormJar = new Button(_jarsGroup, SWT.CHECK);
        _iceStormJar.setText("IceStorm");
    }

    /**
     * @see PreferencePage#createContents(Composite)
     */
    protected Control createContents(Composite parent)
    {
        // Composite composite = new Composite(parent, SWT.NONE);

        TabFolder tabFolder = new TabFolder(parent, SWT.NONE);
        {
            TabItem tabItem = new TabItem(tabFolder, SWT.NONE);
            tabItem.setText("Source");
            Control source = createSource(tabFolder);
            tabItem.setControl(source);
        }
        {
            TabItem tabItem = new TabItem(tabFolder, SWT.NONE);
            tabItem.setText("Options");
            Control source = createOptions(tabFolder);
            tabItem.setControl(source);
        }
        tabFolder.pack();

        loadPrefs();

        return tabFolder;
    }

    private void loadPrefs()
    {
        IProject project = getProject();
        _config = new Configuration(project);

        _generatedDir.setText(_config.getGeneratedDir());
        for(Iterator<String> iter = _config.getSliceSourceDirs().iterator(); iter.hasNext();)
        {
            _sourceDirectories.add(iter.next());
        }
        for(Iterator<String> iter = _config.getBareIncludes().iterator(); iter.hasNext();)
        {
            _includes.add(iter.next());
        }
        for(Iterator<String> iter = _config.getJars().iterator(); iter.hasNext();)
        {
            String jarFile = iter.next();
            if(jarFile.equals("Freeze.jar"))
            {
                _freezeJar.setSelection(true);
            }
            else if(jarFile.equals("Glacier2.jar"))
            {
                _glacier2Jar.setSelection(true);
            }
            else if(jarFile.equals("IceBox.jar"))
            {
                _iceBoxJar.setSelection(true);
            }
            else if(jarFile.equals("IceGrid.jar"))
            {
                _iceGridJar.setSelection(true);
            }
            else if(jarFile.equals("IcePatch2.jar"))
            {
                _icePatch2Jar.equals(true);
            }
            else if(jarFile.equals("IceStorm.jar"))
            {
                _iceStormJar.setSelection(true);
            }
        }
        _defines.setText(Configuration.fromList(_config.getDefines()));
        _meta.setText(Configuration.fromList(_config.getMeta()));
        _stream.setSelection(_config.getStream());
        _tie.setSelection(_config.getTie());
        _ice.setSelection(_config.getIce());
        _underscore.setSelection(_config.getUnderscore());
        _console.setSelection(_config.getConsole());
        _extraArguments.setText(_config.getExtraArguments());

        //
        // Android projects don't support Freeze.
        //
        _freezeJar.setEnabled(!_config.isAndroidProject());

        checkValid();
    }

    private void checkValid()
    {
        IProject project = getProject();
        IFolder folder = project.getFolder(_generatedDir.getText());
        if(!folder.exists())
        {
            setErrorMessage("Generated folder does not exist");
            setValid(false);
            return;
        }
        setValid(true);
        setErrorMessage(null);
    }

    private Control createSource(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        composite.setLayout(gridLayout);

        Group sourceGroup = new Group(composite, SWT.NONE);
        sourceGroup.setText("Location of Slice Source Files");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        sourceGroup.setLayout(gridLayout);
        sourceGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        Composite c1 = new Composite(sourceGroup, SWT.NONE);

        gridLayout = new GridLayout();
        gridLayout.numColumns = 2;
        c1.setLayout(gridLayout);
        c1.setLayoutData(new GridData(GridData.FILL_BOTH));

        _sourceDirectories = new List(c1, SWT.H_SCROLL | SWT.V_SCROLL | SWT.MULTI | SWT.BORDER);
        _sourceDirectories.setLayoutData(new GridData(GridData.FILL_BOTH));

        Composite c2 = new Composite(c1, SWT.NONE);

        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        c2.setLayout(gridLayout);

        Button but1 = new Button(c2, SWT.PUSH);
        but1.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        but1.setText("Add Folder");
        but1.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                IProject project = getProject();

                SourceSelectionDialog dialog = new SourceSelectionDialog(getShell(), project, "Select Source Location");
                String[] items = _sourceDirectories.getItems();
                IFolder[] resources = new IFolder[items.length];
                for(int i = 0; i < items.length; ++i)
                {
                    resources[i] = project.getFolder(items[i]);
                }
                dialog.setInitialSelections(resources);
                if(dialog.open() == ContainerSelectionDialog.OK)
                {
                    Object[] selection = dialog.getResult();
                    for(int i = 0; i < selection.length; ++i)
                    {
                        IFolder path = (IFolder) selection[i];
                        _sourceDirectories.add(path.getProjectRelativePath().toString());
                    }
                }
            }
        });

        Button but2 = new Button(c2, SWT.PUSH);
        but2.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        but2.setText("Remove");
        but2.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                _sourceDirectories.remove(_sourceDirectories.getSelectionIndices());
            }
        });

        Group gclGroup = new Group(composite, SWT.NONE);
        gclGroup.setText("Generated Code Location");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        gclGroup.setLayout(gridLayout);
        gclGroup.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

        Composite tc = new Composite(gclGroup, SWT.NONE);
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        tc.setLayout(gridLayout);
        tc.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

        Label l = new Label(tc, SWT.WRAP);
        l.setForeground(new Color(null, 255, 0, 0));
        l.setText("This subdirectory is used by the plug-in to manage the source files generated from "
                + "your Slice definitions. It should not be used for any other purpose. "
                + "Files added manually are removed during project rebuilds.");

        GridData gridData = new GridData(GridData.FILL_BOTH);
        gridData.widthHint = 400;
        l.setLayoutData(gridData);

        Composite c = new Composite(tc, SWT.NONE);

        GridLayout gridLayout2 = new GridLayout();
        gridLayout2.numColumns = 2;
        gridLayout2.marginLeft = 0;
        gridLayout2.marginTop = 0;
        gridLayout2.marginBottom = 0;
        c.setLayout(gridLayout2);

        c.setLayoutData(new GridData(GridData.FILL_BOTH));

        _generatedDir = new Text(c, SWT.BORDER | SWT.READ_ONLY);
        gridData = new GridData(GridData.FILL_HORIZONTAL);
        // gridData.horizontalSpan = 2;
        _generatedDir.setLayoutData(gridData);

        Button but3 = new Button(c, SWT.PUSH);
        but3.setText("Browse");
        but3.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                IProject project = getProject();

                SourceSelectionDialog dialog = new SourceSelectionDialog(getShell(), project,
                        "Select Generated Code Location");
                dialog.setMultiple(false);
                if(dialog.open() == ContainerSelectionDialog.OK)
                {
                    Object[] selection = dialog.getResult();
                    if(selection.length == 1)
                    {
                        IFolder path = (IFolder) selection[0];
                        String oldPath = _generatedDir.getText();
                        String newPath = path.getProjectRelativePath().toString();
                        if(oldPath.equals(newPath))
                        {
                            return;
                        }
                        try
                        {
                            if(path.members().length > 0)
                            {
                                ErrorDialog.openError(getShell(), "Error",
                                        "Generated code location should be an empty folder", new Status(Status.ERROR,
                                                Activator.PLUGIN_ID, "The chosen directory '"
                                                        + path.getFullPath().toOSString() + "' is not empty."));
                                return;
                            }
                        }
                        catch(CoreException ex)
                        {
                            ErrorDialog.openError(getShell(), "Error", ex.toString(), new Status(Status.ERROR,
                                    Activator.PLUGIN_ID, 0, "Failed to set generated code location.", ex));
                            return;
                        }
                        _generatedDir.setText(newPath);
                    }
                }
            }
        });

        return composite;
    }

    private Button _freezeJar;
    private Button _glacier2Jar;
    private Button _iceBoxJar;
    private Button _iceGridJar;
    private Button _icePatch2Jar;
    private Button _iceStormJar;

    private Text _generatedDir;
    private List _sourceDirectories;
    private Group _jarsGroup;
}
