// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.BorderLayout;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;

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
  
    protected abstract void appendProperties(DefaultFormBuilder builder);

    protected void buildPropertiesPanel()
    {
        FormLayout layout = new FormLayout(
            "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
        
        DefaultFormBuilder builder = new DefaultFormBuilder(layout);
        builder.setBorder(Borders.DLU2_BORDER);
        builder.setRowGroupingEnabled(true);
        builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
        
        appendProperties(builder);
        
        JScrollPane scrollPane = 
            new JScrollPane(builder.getPanel(),
                            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
                            JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        
        scrollPane.setBorder(Borders.DIALOG_BORDER);
        
        _propertiesPanel = new JPanel(new BorderLayout());
        _propertiesPanel.add(scrollPane, BorderLayout.CENTER);
        _propertiesPanel.setBorder(Borders.EMPTY_BORDER);
    }
    
    protected JPanel _propertiesPanel;  
}
