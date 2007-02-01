// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import javax.swing.tree.TreePath;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.factories.DefaultComponentFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;
import IceGridGUI.*;

public class Editor extends EditorBase
{      
    static public java.util.Map makeParameterValues(
        java.util.Map oldParameterValues,
        java.util.List newParameters)
    {
        java.util.Map result = new java.util.HashMap();

        java.util.Iterator p = newParameters.iterator();
        while(p.hasNext())
        {
            Object name =  p.next();
            Object value = oldParameterValues.get(name);
            if(value != null)
            {
                result.put(name, value);
            } 
        }
        return result;
    }

    public boolean save(boolean refresh)
    {
        if(_applyButton.isEnabled())
        {
            return validate() && applyUpdate(refresh);
        }
        else
        {
            return true;
        }
    }

    protected boolean validate()
    {
        return true;
    }

    protected boolean applyUpdate(boolean refresh)
    {
        assert false;
        return false;
    }
    
    protected void detectUpdates(boolean val)
    {
        _detectUpdates = val;
    }

    protected void discardUpdate()
    {
        if(_target.isEphemeral())
        {
            _target.delete();
        }
        else
        {
            _target.getCoordinator().getCurrentTab().showNode(_target);
        }
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {}

    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        JComponent buttonBar = 
            ButtonBarFactory.buildRightAlignedBar(_applyButton, 
                                                  _discardButton);
        buttonBar.setBorder(Borders.DIALOG_BORDER);
        _propertiesPanel.add(buttonBar, BorderLayout.SOUTH);
    }


    Editor()
    {
        //
        // _applyButton
        //
        AbstractAction apply = new AbstractAction("Apply")
            {
                public void actionPerformed(ActionEvent e) 
                {
                    if(validate())
                    {
                        if(applyUpdate(true))
                        {
                            _target.getRoot().getTree().grabFocus();
                        }
                    }
                }
            };
        _applyButton = new JButton(apply);
        _applyButton.setEnabled(false);
        
        //
        // _discardButton
        //
        AbstractAction discard = new AbstractAction("Discard")
            {
                public void actionPerformed(ActionEvent e) 
                {
                    discardUpdate();
                    _target.getRoot().getTree().grabFocus();
                }
            };
        _discardButton = new JButton(discard);
        _discardButton.setEnabled(false);
        
        _updateListener = new DocumentListener() 
            {
                public void changedUpdate(DocumentEvent e)
                {
                    updated();
                }
                
                public void insertUpdate(DocumentEvent e)
                {
                    updated();
                }
                
                public void removeUpdate(DocumentEvent e)
                {
                    updated();
                }
            };
    }

    //
    // Used by the sub-editor (when there is one)
    //
    Object getSubDescriptor()
    {
        return null;
    }

    Utils.Resolver getDetailResolver()
    {
        return null;
    }

    public TreeNode getTarget()
    {
        return _target;
    }
    
    void updated()
    {
        if(_detectUpdates)
        {
            _target.getRoot().disableRegistryUpdates();
            _applyButton.setEnabled(true);
            _discardButton.setEnabled(true);
        }
    }

    DocumentListener getUpdateListener()
    {
        return _updateListener;
    }

    //
    // Check that these 'val' are filled in
    //
    boolean check(String[] nameValArray)
    {
        String emptyFields = "";
        int errorCount = 0;

        for(int i = 1; i < nameValArray.length; i += 2)
        {
            if(nameValArray[i] == null || nameValArray[i].length() == 0)
            {
                errorCount++;
                if(emptyFields.length() > 0)
                {
                    emptyFields += "\n";
                }
                emptyFields += "'" + nameValArray[i - 1] + "'";
            }
        }
        
        if(errorCount > 0)
        {
            String message = errorCount == 1 ?
                emptyFields + " cannot be empty" :
                "The following fields cannot be empty:\n" + emptyFields;

            JOptionPane.showMessageDialog(
                _target.getCoordinator().getMainFrame(),
                message,
                "Validation failed",
                JOptionPane.ERROR_MESSAGE);
        }

        return errorCount == 0;
    }

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;
   
    protected TreeNode _target;
    private boolean _detectUpdates = true;   
}
