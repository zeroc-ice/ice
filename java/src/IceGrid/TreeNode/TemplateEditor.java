// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTree;

import javax.swing.event.DocumentListener;

import javax.swing.tree.TreePath;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

abstract class TemplateEditor extends Editor
{
    /*

    TemplateEditor(boolean service)
    {
	_templateId.addDocumentListener(_updateListener);
	_parameters.addDocumentListener(_updateListener);

	if(service)
	{
	    _subEditor = new ServiceEditor(this);
	}
	else
	{
	    // TODO: ServerEditor
	}
    }
    
    JComponent getComponent()
    {
	if(_panel == null)
	{
	    //
	    // Build everything using JGoodies's DefaultFormBuilder
	    //
	    FormLayout layout = new FormLayout(
		"right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
	    
	    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	    builder.setBorder(Borders.DLU2_BORDER);
	    builder.setRowGroupingEnabled(true);
	    
	    builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
	    
	    builder.append("Template ID");
	    builder.append(_templateId, 3);
	    builder.nextLine();
	    builder.append("Parameters");
	    builder.append(_parameters, 3);
	    builder.nextLine();
	    _subEditor.buildSubPanel(builder);
	 

	    _panel = buildPanel(builder.getPanel());
	}
	return _panel;
    }
    
    protected CommonBase getTarget()
    {
	return _template;
    }

    static private java.util.LinkedList stringToSortedParams(String s)
    {
	java.util.List params = java.util.Arrays.asList(s.getText.split("\s+"));
	java.util.Collections.sort(params);
	return java.util.LinkedList(params);
    }

    static private String stringify(java.util.List params)
    {
	String result = "";
	java.util.Iterator p = params.iterator();
	while(p.hasNext())
	{
	    if(result.equals(""))
	    {
		result = (String)p.next();
	    }
	    else
	    {
		result = result + " " + (String)p.next();
	    }
	}
	return result;
    }

    void writeDescriptor(boolean fullUpdate)
    {
	TemplateDescriptor descriptor = _template.getDescriptor();

	if(fullUpdate)
	{
	    descriptor.descriptor = _instanceEditor.getNewDescriptor();
	}
	else
	{
	    _instanceEditor.writeDescriptor();
	}
	descriptor.params = stringToSortedParams(_parameters.getText());
    }

    TemplateDescriptor getNewDescriptor()
    {
	return new TemplateDescriptor(
	    _instanceEditor.getNewDescriptor(),
	    stringToSortedParams(_parameters.getText()));
    }

    private boolean isUpdateSafe()
    {
	//
	// If parameter change => not safe!
	//
	if(_origParams.equals(_parameters.getText()))
	{
	    return _instanceEditor.isUpdateSafe();
	}
	else
	{
	    return false;
	}
    }

    protected void applyUpdate()
    {
	if(_template.getModel().canUpdate())
	{
	    _template.getModel().disableDisplay();
	    
	    String oldId = _template.getId();
	    String newId = _templateId.getText();

	    try
	    {
		TemplateDescriptor descriptor = _template.getDescriptor();
		
		if(_template.isEphemeral())
		{
		    Templates parent = (Templates)_template.getParent();
		    writeDescriptor(false);
		   
		    if(!parent.addDescriptor(newId, descriptor))
		    {
			JOptionPane.showMessageDialog(
			    _template.getModel().getMainFrame(),
			    "The id '" + newId + "' is already in use",
			    "Duplicate template id",
			    JOptionPane.ERROR_MESSAGE);
		    }
		    _template.destroy();
		    
		    if(!_template.getApplication().applyUpdate())
		    {
			//
			// Restores old display
			//
			parent = (Templates)_templates.getModel()
			    .findNewNode(parent.getPath());
			parent.removeDescriptor(newId);
			try
			{
			    parent.addChild(_adapter, true);
			}
			catch(DuplicateIdException die)
			{
			    assert false;
			}
			_template.setParent(parent);
			_template.getModel().setSelectionPath(
			    _template.getPath());
			return;
		    }
		    else
		    {
			parent = (Templates)_template.getModel()
			    .findNewNode(parent.getPath());
			_template = (Template)parent.findChild(newId);
			_template.getModel()
			    .setSelectionPath(_template.getPath());
		    }
		}
		else if(oldId.equals(newId))
		{
		    if(isSafeUpdate())
		    {
			writeDescriptor(false);
		    }
		    else
		    {
			//
			// Full update
			//
			
			//
			// Save to be able to rollback
			//
			TemplateDescriptor oldDescriptor = null;		  
			try
			{
			    //
			    // Shallow copy
			    //
			    oldDescriptor = (TemplateDescriptor)descriptor.clone();
			}
			catch(CloneNotSupportedException ce)
			{
			    assert false; // impossible
			}
			
			TreePath oldPath = _template.getPath();
			Templates parent = (Templates)_template.getParent();
		    
			writeDescriptor(true);
		    
			if(!_template.getApplication().applyUpdate())
			{
			    //
			    // Restore descriptor
			    // IMPORTANT: keep the same object!
			    //
			    descriptor.descriptor = oldDescriptor.descriptor;
			    descriptor.parameters = oldDescriptor.parameters;
			    
			    //
			    // Need to find new template node!
			    //
			    _template = (Template)_template.getModel().findNewNode(oldPath);
			    _template.getModel().setSelectionPath(_template.getPath());
			    //
			    //
			    // Everything was restored, user must deal with error
			    //
			    return;
			}
			//
			// Else, no renaming, so proper node already selected
			//
		    }
		}
		else
		{
		    //
		    // Renaming
		    //

		    Templates parent = (Templates)_template.getParent();
		    
		    TemplateDescriptor oldDescriptor = descriptor;
		    TemplateDescriptor newDescriptor = getNewDescriptor();
		    
		    if(!parent.addDescriptor(newId, newDescriptor))
		    {
			JOptionPane.showMessageDialog(
			    _template.getModel().getMainFrame(),
			    "The id '" + newId + "' is already in use",
			    "Duplicate template id",
			    JOptionPane.ERROR_MESSAGE);
		    }
		    parent.removeDescriptor(oldId);

		    if(!_template.getApplication().applyUpdate())
		    {
			parent = (Templates)_template.getModel()
			    .findNewNode(parent.getPath());
			
			parent.addDescriptor(oldId, oldDescriptor);

			//
			// Need to find new template node!
			//
			_template = (Template)_template.getModel().findNewNode(oldPath);
			_template.getModel().setSelectionPath(_template.getPath());
			//
			//
			// Everything was restored, user must deal with error
			//
			return;
		    }
		    else
		    {
			_instanceEditor.cascadeDelete(oldId);

			parent = (Templates)_template.getModel()
			    .findNewNode(parent.getPath());
			_template = (Template)parent.findChild(newId);
			_template.getModel()
			    .setSelectionPath(_template.getPath());
		    }
		}

		//
		// Mark modified
		//
		_template.markModified();
		
		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		_template.getModel().enableDisplay();
	    }
	}
	//
	// Otherwise, may wait until other user is done!
	//
    }
    
    
    void show(Template template)
    {
	detectUpdates(false);
	
	_template = template;
	TemplateDescriptor descriptor = _template.getDescriptor();
	
	_templateId.setText(_template.getId());
	_origParams = stringify(_template.parameters);
	_parameters.setText(_origParams);
	_instanceEditor.show();

	_applyButton.setEnabled(_template.isEphemeral());
	_discardButton.setEnabled(_template.isEphemeral());	  
	detectUpdates(true);
    }
    
    TemplateDescriptor getDescriptor()
    {
	return _template.getDescriptor();
    }
    

    private JTextField _templateId = new JTextField(20);
    private JTextField _parameters = new JTextField(20);

    private String _origParams;

    private JPanel _panel;
    private Template _template;
    */
}
