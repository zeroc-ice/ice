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
import java.awt.event.KeyEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class NodeEditor extends Editor
{
    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Node Properties");
    }

    protected void applyUpdate()
    {
	Root root = _target.getRoot();

	root.disableSelectionListener();
	try
	{
	    if(_target.isEphemeral())
	    {
		Nodes nodes = (Nodes)_target.getParent();
		writeDescriptor();
		NodeDescriptor descriptor = (NodeDescriptor)_target.getDescriptor();
		_target.destroy(); // just removes the child
		try
		{
		    nodes.tryAdd(_name.getText(), descriptor);
		}
		catch(UpdateFailedException e)
		{
		    //
		    // Add back ephemeral child
		    //
		    try
		    {
			nodes.insertChild(_target, true);
		    }
		    catch(UpdateFailedException die)
		    {
			assert false;
		    }
		    root.setSelectedNode(_target);

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
		_target = nodes.findChildWithDescriptor(descriptor);
		root.updated();
		root.setSelectedNode(_target);
	    }
	    else if(isSimpleUpdate())
	    {
		writeDescriptor();
		root.updated();
		((Node)_target).getEditable().markModified();
	    }
	    else
	    {
		//
		// Save to be able to rollback
		//
		NodeDescriptor savedDescriptor = ((Node)_target).saveDescriptor();
		writeDescriptor();
	
		//
		// Rebuild node; don't need the backup
		// since it's just one node
		//
		java.util.List editables = new java.util.LinkedList();
			
		try
		{
		    ((Node)_target).rebuild(editables);
		}
		catch(UpdateFailedException e)
		{
		    ((Node)_target) .restoreDescriptor(savedDescriptor);
		    JOptionPane.showMessageDialog(
			root.getCoordinator().getMainFrame(),
			e.toString(),
			"Apply failed",
			JOptionPane.ERROR_MESSAGE);
		    return;
		}
		
		java.util.Iterator p = editables.iterator();
		while(p.hasNext())
		{
		    Editable editable = (Editable)p.next();
		    editable.markModified();
		}

		((Node)_target).getEditable().markModified();
		root.updated();
	    }

	    root.getCoordinator().getCurrentTab().showNode(_target);
	    _applyButton.setEnabled(false);
	    _discardButton.setEnabled(false);
	}
	finally
	{
	    root.enableSelectionListener();
	}	    
    }

    Utils.Resolver getDetailResolver()
    {
	if(_target.getCoordinator().substitute())
	{
	    return _target.getResolver();
	}
	else
	{
	    return null;
	}
    }

    NodeEditor(JFrame parentFrame)
    {
	_name.getDocument().addDocumentListener(_updateListener);
	_name.setToolTipText("Must match the IceGrid.Node.Name property of the desired icegridnode process");
	_description.getDocument().addDocumentListener(_updateListener);
	_description.setToolTipText("An optional description for this node");
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

	_loadFactor.getDocument().addDocumentListener(_updateListener);
	_loadFactor.setToolTipText("<html>A floating point value.<br>"
				   + "When not specified, IceGrid uses 1.0 on all platforms<br>"
				   + "except Windows where it uses 1.0 divided by <i>number of processors</i>.<html>");
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {    
	builder.append("Name");
	builder.append(_name, 3);
	builder.nextLine();

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
	builder.append("Load Factor");
	builder.append(_loadFactor, 3);
	builder.nextLine();
    }
    
    boolean isSimpleUpdate()
    {
	NodeDescriptor descriptor = (NodeDescriptor)_target.getDescriptor();
	return (_variablesMap.equals(descriptor.variables));
    }

    void writeDescriptor()
    {
	NodeDescriptor descriptor = (NodeDescriptor)_target.getDescriptor();
	descriptor.description = _description.getText();
	descriptor.variables = _variablesMap;
	descriptor.loadFactor = _loadFactor.getText();
    }	    
    
    void show(Node node)
    {
	detectUpdates(false);
	_target = node;
	
	Utils.Resolver resolver = getDetailResolver();
	boolean isEditable = (resolver == null);

	_name.setText(_target.getId());
	_name.setEditable(_target.isEphemeral());
	
	NodeDescriptor descriptor = (NodeDescriptor)_target.getDescriptor();

	_description.setText(
	    Utils.substitute(descriptor.description, resolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);

	_variablesMap = descriptor.variables;
	setVariablesField();
	_variablesButton.setEnabled(isEditable);

	_loadFactor.setText(
	    Utils.substitute(descriptor.loadFactor, resolver));
	_loadFactor.setEditable(isEditable);

	_applyButton.setEnabled(node.isEphemeral());
	_discardButton.setEnabled(node.isEphemeral());
	detectUpdates(true);
    }
    
    private void setVariablesField()
    {
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		final Utils.Resolver resolver = getDetailResolver();

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

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);
    private JTextField _variables = new JTextField(20);
    private JButton _variablesButton;
    private TableDialog _variablesDialog;
    private java.util.TreeMap _variablesMap;
    private JTextField _loadFactor = new JTextField(20);
}
