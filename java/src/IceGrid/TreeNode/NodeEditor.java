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

import IceGrid.Model;
import IceGrid.NodeDescriptor;
import IceGrid.NodeInfo;
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

    Utils.Resolver getDetailResolver()
    {
	Node node = (Node)_target;
	if(node.getModel().substitute())
	{
	    return node.getResolver();
	}
	else
	{
	    return null;
	}
    }

    NodeEditor(JFrame parentFrame)
    {
	super(true, true);

	_name.getDocument().addDocumentListener(_updateListener);
	_name.setToolTipText("Must match the IceGrid.Node.Name property of the desired icegridnode process");
	_description.getDocument().addDocumentListener(_updateListener);
	_description.setToolTipText("An optional description for this node");
	_variables.setEditable(false);

	_hostname.setEditable(false);
	_os.setEditable(false);
	_machineType.setEditable(false);
	_loadAverage.setEditable(false);

	Action refresh = new AbstractAction("Refresh")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    Node node = (Node)_target;
		    node.getLoad();
		}
	    };
	refresh.putValue(Action.SHORT_DESCRIPTION, 
			"Fetch the latest values from this IceGrid Node");
	_refreshButton = new JButton(refresh);


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
 
    public JComponent getCurrentStatus(Ice.StringHolder title)
    {
	title.value = "System information";
	return super.getCurrentStatus(title);
    }

    public void refreshCurrentStatus()
    {
	Node node = (Node)_target;
	NodeInfo info = node.getStaticInfo();
	
	if(info == null)
	{
	    _hostname.setText("Unknown");
	    _os.setText("Unknown");
	    _machineType.setText("Unknown");
	    _loadAverageLabel.setText("Load Average");
	    _loadAverage.setText("Unknown");
	}
	else
	{
	    _hostname.setText(info.hostname);
	    _os.setText(info.os + " " + info.release + " " + info.version);
	    _machineType.setText(info.machine + " with " + 
				 info.nProcessors 
				 + " CPU" 
				 + (info.nProcessors >= 2 ? "s" : ""));
	    
	    if(node.isRunningWindows())
	    {
		_loadAverageLabel.setText("CPU Usage");
		_loadAverage.setToolTipText(
		    "CPU usage in the past 1 min, 5 min and 15 min period");
	    }
	    else
	    {
		_loadAverageLabel.setText("Load Average");
		_loadAverage.setToolTipText(
		    "Load average in the past 1 min, 5 min and 15 min period");
	    }
	    _loadAverage.setText("Refreshing...");
	    node.getLoad();
	}
    }

    void setLoad(String load, Node node)
    {
	if(node == _target)
	{
	    _loadAverage.setText(load);
	}
	//
	// Otherwise, we've already moved to another node
	// 
    }


    void appendCurrentStatus(DefaultFormBuilder builder)
    {
	builder.append("Hostname");
	builder.append(_hostname, 3);
	builder.nextLine();
	builder.append("Operating System");
	builder.append(_os, 3);
	builder.nextLine();
	builder.append("Machine Type");
	builder.append(_machineType, 3);
	builder.append(_loadAverageLabel, _loadAverage);
	builder.append(_refreshButton);
	builder.nextLine();
    }

    void appendProperties(DefaultFormBuilder builder)
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
	setTarget(node);
	
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

	refreshCurrentStatus();
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

    private JTextField _hostname = new JTextField(20);
    private JTextField _os = new JTextField(20);
    private JTextField _machineType = new JTextField(20);
    private JLabel _loadAverageLabel = new JLabel();
    private JTextField _loadAverage = new JTextField(20);
    private JButton _refreshButton;
}
