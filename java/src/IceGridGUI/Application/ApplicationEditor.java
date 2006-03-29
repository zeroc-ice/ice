// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class ApplicationEditor extends Editor
{
    protected void applyUpdate()
    {
	Root root = (Root)_target;
	MainPane mainPane = _target.getCoordinator().getMainPane();

	root.disableSelectionListener();
	try
	{
	    if(root.isEphemeral())
	    {
		writeDescriptor();
		ApplicationDescriptor descriptor = 
		    (ApplicationDescriptor)root.getDescriptor();
		try
		{
		    Root newRoot = new Root(root);
		    ApplicationPane app = mainPane.findApplication(root);
		    app.setRoot(newRoot);
		    newRoot.setSelectedNode(newRoot);
		    app.showNode(newRoot);
		    
		    _target = newRoot;
		    if(!root.getId().equals(newRoot.getId()))
		    {
			mainPane.resetTitle(newRoot);
		    } 
		}
		catch(UpdateFailedException e)
		{
		    JOptionPane.showMessageDialog(
			root.getCoordinator().getMainFrame(),
			e.toString(),
			"Apply failed",
			JOptionPane.ERROR_MESSAGE);
		    return;
		}
	    }
	    else if(isSimpleUpdate())
	    {
		writeDescriptor();
		root.updated();
		root.getEditable().markModified();
	    }
	    else
	    {
		//
		// Save to be able to rollback
		//
		ApplicationDescriptor savedDescriptor = root.saveDescriptor();
		writeDescriptor();
		try
		{
		    root.rebuild();
		}
		catch(UpdateFailedException e)
		{
		    root.restoreDescriptor(savedDescriptor);
		    JOptionPane.showMessageDialog(
			root.getCoordinator().getMainFrame(),
			e.toString(),
			"Apply failed",
			JOptionPane.ERROR_MESSAGE);
		    return;
		}
		//
		// Success
		//
		root.updated();
		root.getEditable().markModified();
		
		if(!savedDescriptor.name.equals(root.getId()))
		{
		    mainPane.resetTitle(root);
		}
	    }
	      
	    _applyButton.setEnabled(false);
	    _discardButton.setEnabled(false);
	}
	finally
	{
	    root.enableSelectionListener();
	}
    }

    ApplicationEditor(JFrame parentFrame)
    {
	_name.getDocument().addDocumentListener(_updateListener);
	_description.getDocument().addDocumentListener(_updateListener);
	_variables.setEditable(false);
	
	//
	// Variables
	//
	_variablesDialog = new TableDialog(parentFrame, 
					   "Variables", "Name", "Value", true);
	
	Action openVariablesDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.TreeMap result = _variablesDialog.show(_variablesMap, 
								     getProperties());
		    if(result != null)
		    {
			updated();
			_variablesMap = result;
			setVariablesField();
		    }
		}
	    };
	openVariablesDialog.putValue(Action.SHORT_DESCRIPTION, 
				     "Edit variables");
	_variablesButton = new JButton(openVariablesDialog);

	//
	// Distrib
	//
	_distrib = new JComboBox(new Object[]{NO_DISTRIB, DEFAULT_DISTRIB});
	_distrib.setEditable(true);
	_distrib.setToolTipText("The proxy to the IcePatch2 server holding your files");

	JTextField distribTextField = (JTextField)
	    _distrib.getEditor().getEditorComponent();
	distribTextField.getDocument().addDocumentListener(
	    _updateListener);

	_distribDirs.setEditable(false);
	_distribDirsDialog = new ListDialog(parentFrame, 
					    "Directories", true);

	Action openDistribDirsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _distribDirsDialog.show(
			_distribDirsList, getProperties());
		    if(result != null)
		    {
			updated();
			_distribDirsList = result;
			setDistribDirsField();
		    }
		}
	    };
	openDistribDirsDialog.putValue(Action.SHORT_DESCRIPTION, 
				       "Edit directory list");
	_distribDirsButton = new JButton(openDistribDirsDialog);

    }
 
    protected void appendProperties(DefaultFormBuilder builder)
    {    
	builder.append("Name");
	builder.append(_name, 3);

	builder.append("Description");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-2);
	CellConstraints cc = new CellConstraints();
	JScrollPane scrollPane = new JScrollPane(_description);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
	builder.nextRow(2);
	builder.nextLine();

	builder.append("Variables", _variables);
	builder.append(_variablesButton);
	builder.nextLine();

	JComponent c = builder.appendSeparator("Distribution");
	c.setToolTipText("Files shared by all servers in your application");
	builder.append("IcePatch2 Proxy");
	builder.append(_distrib, 3);
	builder.nextLine();
	builder.append("Directories");
	builder.append(_distribDirs, _distribDirsButton);
	builder.nextLine();
    }
    
    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Application Properties");
    }

    boolean isSimpleUpdate()
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)_target.getDescriptor();
	return descriptor.name.equals(_name.getText()) && _variablesMap.equals(descriptor.variables);
    }

    void writeDescriptor()
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)_target.getDescriptor();
	descriptor.name = _name.getText();
	descriptor.variables = _variablesMap;
	descriptor.description = _description.getText();

	if(_distrib.getSelectedItem() == NO_DISTRIB)
	{
	    descriptor.distrib.icepatch = "";
	}
	else
	{
	    descriptor.distrib.icepatch = _distrib.getSelectedItem().toString();
	}
	descriptor.distrib.directories = _distribDirsList;
    }	    
    

    void show(Root root)
    {
	detectUpdates(false);
	_target = root;
	
	Utils.Resolver resolver = getDetailResolver();
	boolean isEditable = (resolver == null);

	ApplicationDescriptor descriptor = 
	    (ApplicationDescriptor)root.getDescriptor();

	_name.setText(descriptor.name);
	_name.setEditable(root.isEphemeral() || !root.isLive());

	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
	_description.setToolTipText("An optional description for this application");
	
	_variablesMap = descriptor.variables;
	setVariablesField();
	_variablesButton.setEnabled(isEditable);

	_distrib.setEnabled(true);
	_distrib.setEditable(true);
	String icepatch = 
	    Utils.substitute(descriptor.distrib.icepatch, resolver);
	if(icepatch.equals(""))
	{
	    _distrib.setSelectedItem(NO_DISTRIB);
	}
	else
	{
	    _distrib.setSelectedItem(icepatch);
	}
	_distrib.setEnabled(isEditable);
	_distrib.setEditable(isEditable);

	_distribDirsList = new java.util.LinkedList(descriptor.distrib.directories);
	setDistribDirsField();
	_distribDirsButton.setEnabled(isEditable);

	_applyButton.setEnabled(root.isEphemeral());
	_discardButton.setEnabled(root.isEphemeral());	  
	detectUpdates(true);
    }
    
    private void setDistribDirsField()
    {
	final Utils.Resolver resolver = getDetailResolver();

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substitute((String)obj, resolver);
		}
	    };
	
	_distribDirs.setText(
	    Utils.stringify(_distribDirsList, stringifier, ", ", 
			    toolTipHolder));

	String toolTip = "<html>Include only these directories";

	if(toolTipHolder.value != null)
	{
	    toolTip += ":<br>" + toolTipHolder.value;
	}
	toolTip += "</html>";
	_distribDirs.setToolTipText(toolTip);
    }

    private void setVariablesField()
    {
	final Utils.Resolver resolver = getDetailResolver();

	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return (String)entry.getKey() + "="
			+ Utils.substitute((String)entry.getValue(), resolver);
		}
	    };

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	_variables.setText(
	    Utils.stringify(_variablesMap.entrySet(), stringifier, 
			    ", ", toolTipHolder));
	_variables.setToolTipText(toolTipHolder.value);
    }
    
    static private final Object NO_DISTRIB = new Object()
	{
	    public String toString()
	    {
		return "None selected";
	    }
	};
    static private final String DEFAULT_DISTRIB = "${application}.IcePatch2/server";

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    private JTextField _variables = new JTextField(20);
    private JButton _variablesButton;
    private TableDialog _variablesDialog;
    private java.util.TreeMap _variablesMap;
    
    private JComboBox _distrib;
    private JTextField _distribDirs = new JTextField(20);
    private java.util.LinkedList _distribDirsList;
    private ListDialog _distribDirsDialog;
    private JButton _distribDirsButton;
}
