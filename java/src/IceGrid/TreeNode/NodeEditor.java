// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.NodeDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class NodeEditor extends Editor
{
    protected void applyUpdate()
    {
	Node node = (Node)_target;
	Model model = node.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(node.isEphemeral())
		{
		    Nodes nodes = (Nodes)node.getParent();
		    writeDescriptor();
		    NodeDescriptor descriptor = (NodeDescriptor)node.getDescriptor();
		    node.destroy(); // just removes the child
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
			    node.addChild(node, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(node.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }

		    //
		    // Success
		    //
		    _target = nodes.findChildWithDescriptor(descriptor);
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		    node.markModified();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = node.saveDescriptor();
		    writeDescriptor();
		    try
		    {
			if(node.inRegistry())
			{
			    //
			    // Rebuild node; don't need the backup
			    // since it's just one node
			    //
			    java.util.List editables = 
				new java.util.LinkedList();

			    node.rebuild(editables);
			    java.util.Iterator p = editables.iterator();
			    while(p.hasNext())
			    {
				Editable editable = (Editable)p.next();
				editable.markModified();
			    }
			    node.markModified();
			}
			else
			{
			    node.moveToRegistry();
			}
		    }
		    catch(UpdateFailedException e)
		    {
			node.restoreDescriptor(savedDescriptor);
			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }
		    //
		    // Success
		    //
		    model.showActions(_target);
		}
		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	}
    }

    NodeEditor(JFrame parentFrame)
    {
	_name.getDocument().addDocumentListener(_updateListener);
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
								     getPanel());
		    if(result != null)
		    {
			updated();
			_variablesMap = result;
			setVariablesField();
		    }
		}
	    };
	_variablesButton = new JButton(openVariablesDialog);

	_loadFactor.getDocument().addDocumentListener(_updateListener);
    }
 
    void append(DefaultFormBuilder builder)
    {    
	builder.append("Name");
	builder.append(_name, 3);
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
	descriptor.variables = _variablesMap;
	descriptor.loadFactor = _loadFactor.getText();
    }	    
    
    void show(Node node)
    {
	detectUpdates(false);
	setTarget(node);
	
	_name.setText(_target.getId());
	_name.setEditable(_target.isEphemeral());
	
	NodeDescriptor descriptor = (NodeDescriptor)_target.getDescriptor();

	if(descriptor == null)
	{
	    _variablesMap = new java.util.TreeMap();
	    setVariablesField();
	    _loadFactor.setText("");
	}
	else
	{
	    _variablesMap = descriptor.variables;
	    setVariablesField();
	    
	    _loadFactor.setText(descriptor.loadFactor);
	}
 
	_applyButton.setEnabled(node.isEphemeral());
	_discardButton.setEnabled(node.isEphemeral());
	detectUpdates(true);
    }

    private void setVariablesField()
    {
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	_variables.setText(
	    Utils.stringify(_variablesMap, "=", ", ", toolTipHolder));
	_variables.setToolTipText(toolTipHolder.value);
    }

    
    private JTextField _name = new JTextField(20);
    private JTextField _variables = new JTextField(20);
    private JButton _variablesButton;
    private TableDialog _variablesDialog;
    private java.util.TreeMap _variablesMap;
    private JTextField _loadFactor = new JTextField(20);

}
