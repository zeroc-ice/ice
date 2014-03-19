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
import java.util.Arrays;
import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IncrementalProjectBuilder;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Status;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.jdt.core.IJavaProject;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.DirectoryDialog;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Text;

import com.zeroc.slice2javaplugin.Activator;
import com.zeroc.slice2javaplugin.builder.Slice2JavaBuilder;
import com.zeroc.slice2javaplugin.internal.Configuration;

public abstract class PropertyPage extends org.eclipse.ui.dialogs.PropertyPage
{
    public PropertyPage(boolean projectPage)
    {
        super();
        _projectPage = projectPage;
        noDefaultAndApplyButton();
    }

    public void performApply()
    {
        super.performApply();
    }
    
    protected Control createIncludes(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 2;
        composite.setLayout(gridLayout);
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));

        _includes = new List(composite, SWT.H_SCROLL | SWT.V_SCROLL | SWT.MULTI | SWT.BORDER);
        _includes.setLayoutData(new GridData(GridData.FILL_BOTH));

        Composite c2 = new Composite(composite, SWT.NONE);

        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        c2.setLayout(gridLayout);

        Button but1 = new Button(c2, SWT.PUSH);
        but1.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        but1.setText("Add");
        but1.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                IProject project = getProject();
                DirectoryDialog dialog = new DirectoryDialog(getShell());
                String dir = dialog.open();
                if(dir != null)
                {
                    IPath projectLocation = project.getLocation();
                    IPath includeLocation = new Path(dir);
                    String dev1 = projectLocation.getDevice();
                    if(dev1 == null)
                    {
                        dev1 = "";
                    }
                    String dev2 = includeLocation.getDevice();
                    if(dev2 == null)
                    {
                        dev2 = "";
                    }
                    IPath result;

                    // If the directories are on different devices, then we have
                    // no choice but to use an absolute path.
                    if(!dev1.equals(dev2))
                    {
                        result = includeLocation;
                    }
                    else
                    {

                        // Convert the absolute path to a relative path.
                        int n = projectLocation.matchingFirstSegments(includeLocation);
                        result = includeLocation.removeFirstSegments(n);

                        IPath up = new Path("..");
                        for(n = projectLocation.segmentCount() - n; n > 0; --n)
                        {
                            result = up.append(result);
                        }
                        // The devices must match, so remove it.
                        result = result.setDevice(null);
                    }
                    _includes.add(result.toString());
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
                _includes.remove(_includes.getSelectionIndices());
            }
        });
        Button but3 = new Button(c2, SWT.PUSH);
        but3.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        but3.setText("Up");
        but3.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                int index = _includes.getSelectionIndex();
                if(index > 0)
                {
                    String[] items = _includes.getItems();
                    String tmp = items[index-1];
                    items[index-1] = items[index];
                    items[index] = tmp;
                    _includes.setItems(items);
                    _includes.setSelection(index-1);
                }
            }
        });
        Button but4 = new Button(c2, SWT.PUSH);
        but4.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        but4.setText("Down");
        but4.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                int index = _includes.getSelectionIndex();
                if(index != -1)
                {
                    String[] items = _includes.getItems();
                    if(index != items.length-1)
                    {
                        String tmp = items[index+1];
                        items[index+1] = items[index];
                        items[index] = tmp;
                        _includes.setItems(items);
                        _includes.setSelection(index+1);
                    }
                }
            }
        });

        return composite;
    }

    private String semiFilter(String text)
    {
        java.util.List<String> l = Arrays.asList(text.split(";"));
        StringBuffer sb = new StringBuffer();
        for(Iterator<String> p = l.iterator(); p.hasNext();)
        {
            String n = p.next().trim();
            if(n.length() > 0)
            {
                if(sb.length() != 0)
                {
                    sb.append(';');
                }
                sb.append(n);
            }
        }
        return sb.toString();
    }

    protected Control createDefines(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        gridLayout.marginLeft = 0;
        gridLayout.marginTop = 0;
        gridLayout.marginBottom = 0;
        composite.setLayout(gridLayout);
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));

        Label l = new Label(composite, SWT.WRAP);
        l.setText("Enter macros (';' separated). For example, enter FOO;BAR to define -DFOO -DBAR.");
        GridData gridData = new GridData(GridData.FILL_BOTH);
        gridData.widthHint = 400;
        l.setLayoutData(gridData);

        _defines = new Text(composite, SWT.BORDER);
        _defines.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        _defines.addFocusListener(new FocusListener()
        {
            public void focusGained(FocusEvent e)
            {
            }

            public void focusLost(FocusEvent e)
            {
                Text t = (Text)e.widget;
                String f = t.getText();
                String filtered = semiFilter(f);
                if(!f.equals(filtered))
                {
                    t.setText(filtered);
                }
            }
        });

        return composite;
    }

    protected Control createMeta(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        gridLayout.marginLeft = 0;
        gridLayout.marginTop = 0;
        gridLayout.marginBottom = 0;
        composite.setLayout(gridLayout);
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));

        Label l = new Label(composite, SWT.WRAP);
        l.setText("Enter metadata (';' separated). For example, enter as:package:com.acme to define --meta=as:package:com.acme.");
        GridData gridData = new GridData(GridData.FILL_BOTH);
        gridData.widthHint = 400;
        l.setLayoutData(gridData);

        _meta = new Text(composite, SWT.BORDER);
        _meta.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        _meta.addFocusListener(new FocusListener()
        {
            public void focusGained(FocusEvent e)
            {
            }

            public void focusLost(FocusEvent e)
            {
                Text t = (Text)e.widget;
                String f = t.getText();
                String filtered = semiFilter(f);
                if(!f.equals(filtered))
                {
                    t.setText(filtered);
                }
            }
        });

        return composite;
    }
    
    protected void createPreOptions(Composite parent)
    {
    }
    
    protected void createPostOptions(Composite parent)
    {
    }

    protected Control createOptions(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        composite.setLayout(gridLayout);
        
        createPreOptions(composite);

        Group includesGroup = new Group(composite, SWT.NONE);
        includesGroup.setText("Location of Include Files");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        includesGroup.setLayout(gridLayout);
        includesGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        createIncludes(includesGroup);

        Group definesGroup = new Group(composite, SWT.NONE);
        definesGroup.setText("Preprocessor Definitions");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        definesGroup.setLayout(gridLayout);
        definesGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        createDefines(definesGroup);

        Group metaGroup = new Group(composite, SWT.NONE);
        metaGroup.setText("Metadata Definitions");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        metaGroup.setLayout(gridLayout);
        metaGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        createMeta(metaGroup);

        Group optionsGroup = new Group(composite, SWT.NONE);

        gridLayout = new GridLayout();
        gridLayout.numColumns = 5;
        optionsGroup.setText("Options");
        optionsGroup.setLayout(gridLayout);
        optionsGroup.setLayoutData(new GridData(GridData.FILL_BOTH));

        _stream = new Button(optionsGroup, SWT.CHECK);
        _stream.setText("Enable streaming");
        _tie = new Button(optionsGroup, SWT.CHECK);
        _tie.setText("Enable tie");
        _ice = new Button(optionsGroup, SWT.CHECK);
        _ice.setText("Enable ice");
        if(_projectPage)
        {
            _console = new Button(optionsGroup, SWT.CHECK);
            _console.setText("Enable console");
        }
        _underscore = new Button(optionsGroup, SWT.CHECK);
        _underscore.setText("Enable underscore");
        
        Group extraArgumentsGroup = new Group(composite, SWT.NONE);
        extraArgumentsGroup.setText("Extra Compiler Arguments");
        gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        extraArgumentsGroup.setLayout(gridLayout);
        extraArgumentsGroup.setLayoutData(new GridData(GridData.FILL_BOTH));
        createExtraArguments(extraArgumentsGroup);
        
        createPostOptions(composite);
        
        return composite;
    }
    
    public Control createExtraArguments(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        gridLayout.marginLeft = 0;
        gridLayout.marginTop = 0;
        gridLayout.marginBottom = 0;
        composite.setLayout(gridLayout);
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));

        Label l = new Label(composite, SWT.WRAP);
        l.setText("Enter extra arguments for the Slice compiler, such as --checksum=foo.Bar.SliceChecksums. " +
                  "These arguments are appended to the compiler command line.");
        GridData gridData = new GridData(GridData.FILL_BOTH);
        gridData.widthHint = 400;
        l.setLayoutData(gridData);
        
        _extraArguments = new Text(composite, SWT.BORDER);
        _extraArguments.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        return composite;
    }

    protected IProject getProject()
    {
        IAdaptable a = getElement();
        if(a instanceof IProject)
        {
            return (IProject)a;
        }
        else if(a instanceof IJavaProject)
        {
            return ((IJavaProject) a).getProject();
        }
        else if(a instanceof IResource)
        {
            return ((IResource)a).getProject();
        }
        else
        {
            assert(false);
            return null;
        }
    }
    
    protected IResource getResource()
    {
        IAdaptable a = getElement();
        if(a instanceof IResource)
        {
            return (IResource)a;
        }
        else
        {
            assert(false);
            return null;
        }
    }
    
    protected boolean configSaveAndRebuild()
    {
        final IProject project = getProject();
        try
        {
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
            ErrorDialog.openError(getShell(), "Error", "Error saving preferences",
                    new Status(Status.ERROR, Activator.PLUGIN_ID, 0, null, e));
            return false;
        }
        return true;
    }
    
    protected Configuration _config;
    protected Button _console;
    protected List _includes;

    protected Text _defines;
    protected Button _stream;
    protected Button _tie;
    protected Button _ice;
    protected Button _underscore;
    protected Text _meta;
    protected Text _extraArguments;
    protected boolean _projectPage;
}
