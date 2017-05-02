// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI;

import java.awt.BorderLayout;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;
import javax.swing.ScrollPaneConstants;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

//
// Base class for all editors
//
public abstract class EditorBase
{
    public JComponent getProperties()
    {
        if(_propertiesPanel == null)
        {
            buildPropertiesPanel();
        }
        return _propertiesPanel;
    }

    public JToolBar getToolBar()
    {
        return null;
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
    }

    protected JComponent createPropertiesPanel()
    {
        return null;
    }

    protected void buildPropertiesPanel()
    {
        JComponent contents = createPropertiesPanel();

        if(contents == null)
        {
            FormLayout layout = new FormLayout("right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");

            DefaultFormBuilder builder = new DefaultFormBuilder(layout);
            builder.border(Borders.DLU2);
            builder.rowGroupingEnabled(true);
            builder.lineGapSize(LayoutStyle.getCurrent().getLinePad());

            appendProperties(builder);

            contents = builder.getPanel();
        }

        JScrollPane scrollPane =
            new JScrollPane(contents,
                            ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                            ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        scrollPane.setBorder(Borders.DIALOG);

        if(_propertiesPanel == null)
        {
            _propertiesPanel = new JPanel(new BorderLayout());
        }
        else
        {
            _propertiesPanel.removeAll();
        }
        _propertiesPanel.add(scrollPane, BorderLayout.CENTER);
        _propertiesPanel.setBorder(Borders.EMPTY);
        _propertiesPanel.revalidate();
    }

    protected JPanel _propertiesPanel;
}
