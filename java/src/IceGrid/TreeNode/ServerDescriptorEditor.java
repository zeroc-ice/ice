// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGrid.TreeNode;

import IceGrid.ServerDescriptor;
import IceGrid.IceBoxDescriptor;
import IceGrid.PropertyDescriptor;
import IceGrid.Utils;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.JTextArea;
import javax.swing.JLabel;
import javax.swing.JScrollPane;


import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.forms.util.LayoutStyle;

class ServerDescriptorEditor
{
    void build(DefaultFormBuilder builder)
    {	
	builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());

	builder.appendSeparator();
	builder.append("Name");
	builder.append(_name, 3);
	builder.nextLine();

	builder.append("Description");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-2);
	CellConstraints cc = new CellConstraints();
	JScrollPane scrollPane = new JScrollPane(_comment);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
	builder.nextRow(2);
	builder.nextLine();
	
	builder.appendSeparator("Configuration");
	builder.append("Properties");
	if(_editable)
	{
	    builder.append(_properties, new JButton("..."));
	}
	else
	{
	    builder.append(_properties, 3);
	}
	builder.nextLine();
	
	builder.append(_endpointsLabel);
	builder.append(_endpoints, 3);
	builder.nextLine();


	builder.appendSeparator("Process Launch");

	builder.append("Activation Mode");
	builder.append(_activation, 3);

	builder.nextLine();

	builder.append("Command");
	if(_editable)
	{
	    builder.append(_exe, new JButton("..."));
	}
	else
	{
	    builder.append(_exe, 3);
	}
	builder.nextLine();

	builder.append("Command Arguments");
	if(_editable)
	{
	    builder.append(_options, new JButton("..."));
	}
	else
	{
	    builder.append(_options, 3);
	}
	builder.nextLine();

	builder.append("Working Directory");
	if(_editable)
	{
	    builder.append(_pwd, new JButton("..."));
	}
	else
	{
	    builder.append(_pwd, 3);
	}
	builder.nextLine();

	builder.append("Environment Variables");
	if(_editable)
	{
	    builder.append(_envs, new JButton("..."));
	}
	else
	{
	    builder.append(_envs, 3);
	}
	builder.nextLine();

	
	if(!_editable)
	{
	    //
	    // All the components are just for show
	    //
	    _name.setEditable(false);
	    _comment.setEditable(false);
	    _comment.setOpaque(false);
	  
	    _properties.setEditable(false);
	    _endpoints.setEditable(false);

	    _activation.setEditable(false);
	    _exe.setEditable(false);
	    _options.setEditable(false);
	    _pwd.setEditable(false);
	    _envs.setEditable(false);
	}
    }

    void show(ServerDescriptor descriptor, final java.util.Map[] variables)
    {
	//
	// Can't edit with substitutions
	//
	if(variables != null)
	{
	    assert(!_editable);
	}

	//
	// Unfortunately this does not resize the panel;
	// not a big deal for just one line
	//
	if(descriptor instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)descriptor;
	}
	else
	{
	    _endpoints.setText("");
	    _endpointsLabel.setEnabled(false);
	    _endpoints.setEnabled(false);
	}
		
	_name.setText(
	    Utils.substituteVariables(descriptor.id, variables));
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();

	Utils.Stringifier propertyStringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    PropertyDescriptor pd = (PropertyDescriptor)obj;
		    return Utils.substituteVariables(pd.name, variables) 
			+ "=" 
			+ Utils.substituteVariables(pd.value, variables);
		}
	    };

	_properties.setText(Utils.stringify(descriptor.properties, propertyStringifier,
					    ", ", toolTipHolder));
	_properties.setToolTipText(toolTipHolder.value);

	_comment.setText(
	    Utils.substituteVariables(descriptor.description, variables));
	_exe.setText(
	    Utils.substituteVariables(descriptor.exe, variables));

	Utils.Stringifier eltStringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substituteVariables((String)obj, variables); 
		}
	    };


	_options.setText(Utils.stringify(
			     java.util.Arrays.asList(descriptor.options), 
			     eltStringifier,
			     " ", toolTipHolder));
	_options.setToolTipText(toolTipHolder.value);
			    
	_envs.setText(Utils.stringify(
			  java.util.Arrays.asList(descriptor.envs), 
			  eltStringifier,
			  " ", toolTipHolder));
	_envs.setToolTipText(toolTipHolder.value);

	_activation.setText(
		Utils.substituteVariables(descriptor.activation, variables));
    }
    

    ServerDescriptorEditor(boolean editable)
    {
	_editable = editable;
    }

    //
    // Editor or simple read-only renderer?
    //
    private boolean _editable;

    //
    // ComponentDescriptor
    //
    private JTextField _name = new JTextField(20);
    private JTextField _properties = new JTextField(20);
    private JTextArea _comment = new JTextArea(3, 20);

    //
    // ServerDescriptor
    //
    private JTextField _exe = new JTextField(20);
    private JTextField _pwd = new JTextField(20);
    private JTextField _options = new JTextField(20);
    private JTextField _envs = new JTextField(20);

    private JTextField _activation = new JTextField("20");

    //
    // IceBoxDescriptor
    //
    private JLabel _endpointsLabel = new JLabel("IceBox Endpoints");
    private JTextField _endpoints = new JTextField(20);
};
