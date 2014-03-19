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

import java.util.Arrays;
import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.TabFolder;
import org.eclipse.swt.widgets.TabItem;
import org.eclipse.swt.widgets.Text;

import com.zeroc.slice2javaplugin.internal.Configuration;

public class SliceFilePropertyPage extends PropertyPage 
{
    public SliceFilePropertyPage()
    {
        super(false);
        setTitle("Slice2as Settings");
    }

    protected void createPreOptions(Composite parent)
    {
        Composite composite = new Composite(parent, SWT.NONE);

        GridLayout gridLayout = new GridLayout();
        gridLayout.numColumns = 1;
        gridLayout.marginLeft = 0;
        gridLayout.marginTop = 0;
        gridLayout.marginBottom = 0;
        composite.setLayout(gridLayout);
        composite.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

        Label l = new Label(composite, SWT.WRAP);
        l.setText("File:");
        GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
        l.setLayoutData(gridData);
                
        Text pathValueText = new Text(composite, SWT.WRAP | SWT.READ_ONLY);
        pathValueText.setText(getResource().getFullPath().toString());
        pathValueText.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
    }

    protected Control createContents(Composite parent)
    {
        TabFolder tabFolder = new TabFolder(parent, SWT.NONE);
        {
            TabItem tabItem = new TabItem(tabFolder, SWT.NONE);
            
            
            Control source = createOptions(tabFolder);
            tabItem.setText("Options");
            tabItem.setControl(source);
        }
        
        tabFolder.pack();
        loadPrefs();
        return tabFolder;
    }
    
    public boolean performOk()
    {
        final IResource resource = getResource();
        _config.setIncludes(resource, Arrays.asList(_includes.getItems()));
        _config.setDefines(resource, Configuration.toList(_defines.getText()));
        _config.setStream(_stream.getSelection());
        _config.setMeta(resource, Configuration.toList(_meta.getText()));
        _config.setTie(resource, _tie.getSelection());
        _config.setIce(resource, _ice.getSelection());
        _config.setUnderscore(resource, _underscore.getSelection());
        _config.setExtraArguments(resource, _extraArguments.getText());
        return configSaveAndRebuild();
    }
    
    private void loadPrefs()
    {
        IProject project = getProject();
        IResource resource = getResource();
        _config = new Configuration(project);

        for(Iterator<String> iter = _config.getBareIncludes(resource).iterator(); iter.hasNext();)
        {
            _includes.add(iter.next());
        }

        _defines.setText(Configuration.fromList(_config.getDefines(resource)));
        _meta.setText(Configuration.fromList(_config.getMeta(resource)));
        _stream.setSelection(_config.getStream());
        _tie.setSelection(_config.getTie(resource));
        _ice.setSelection(_config.getIce(resource));
        _underscore.setSelection(_config.getUnderscore(resource));
        _extraArguments.setText(_config.getExtraArguments(resource));
    }
}
