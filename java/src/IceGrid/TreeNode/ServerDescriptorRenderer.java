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

import javax.swing.JTextField;
import javax.swing.JTextArea;
import javax.swing.JLabel;
import javax.swing.JScrollPane;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

class ServerDescriptorRenderer
{
    void build(DefaultFormBuilder builder)
    {
	builder.appendSeparator("Server Descriptor");
	builder.append("Name");
	builder.append(_name, 3);
	builder.nextLine();
	builder.append("Properties");
	builder.append(_properties, 3);
	builder.nextLine();
	builder.append("Comment");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-2);
	CellConstraints cc = new CellConstraints();
	JScrollPane scrollPane = new JScrollPane(_comment);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
	builder.nextRow(2);
	builder.nextLine();
	builder.append("Command");
	builder.append(_exe, 3);
	builder.nextLine();
	builder.append("Command Arguments");
	builder.append(_options, 3);
	builder.nextLine();
	builder.append("Working Directory");
	builder.append(_pwd, 3);
	builder.nextLine();
	builder.append("Interpreter");
	builder.append(_interpreter, 3);
	builder.nextLine();
	builder.append("Interpreter Options");
	builder.append(_interpreterOptions, 3);
	builder.nextLine();
	builder.append("Environment Variables");
	builder.append(_envs, 3);
	builder.nextLine();
	builder.append("Activation Mode");
	builder.append(_activation, 3);
	builder.nextLine();
	builder.append(_endpointsLabel);
	builder.append(_endpoints, 3);

	//
	// Make every component non-editable (they are just for show)
	//
	_name.setEditable(false);
	_properties.setEditable(false);
	_comment.setEditable(false);
	_comment.setOpaque(false);
	_exe.setEditable(false);
	_options.setEditable(false);
	_pwd.setEditable(false);
	_interpreter.setEditable(false);
	_interpreterOptions.setEditable(false);
	_envs.setEditable(false);
	_activation.setEditable(false);
	_endpoints.setEditable(false);
    }

    void showDescriptor(ServerDescriptor descriptor)
    {
	//
	// Unfortunately this does not resize the panel;
	// not a big deal for just one line
	//
	if(descriptor instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)descriptor;
	    _endpoints.setText(iceBoxDescriptor.endpoints);
	    _endpointsLabel.setVisible(true);
	    _endpoints.setVisible(true);
	}
	else
	{
	    _endpointsLabel.setVisible(false);
	    _endpoints.setVisible(false);
	}
		
	_name.setText(descriptor.name);
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();

	Utils.Stringifier propertyStringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    PropertyDescriptor pd = (PropertyDescriptor)obj;
		    return pd.name + "=" + pd.value;
		}
	    };

	_properties.setText(Utils.stringify(descriptor.properties, propertyStringifier,
					    ", ", toolTipHolder));
	_properties.setToolTipText(toolTipHolder.value);

	_comment.setText(descriptor.comment);
	_exe.setText(descriptor.exe);

	_options.setText(Utils.stringify(descriptor.options, " ", toolTipHolder));
	_options.setToolTipText(toolTipHolder.value);
			    
	_interpreter.setText(descriptor.interpreter);
	
	_interpreterOptions.setText(Utils.stringify(descriptor.interpreterOptions, " ", toolTipHolder));
	_interpreterOptions.setToolTipText(toolTipHolder.value);


	_envs.setText(Utils.stringify(descriptor.envs, " ", toolTipHolder));
	_envs.setToolTipText(toolTipHolder.value);

	_activation.setText(descriptor.activation);
    }
    
    //
    // ComponentDescriptor
    //
    private JTextField _name = new JTextField(20);
    private JTextField _variables = new JTextField(20);
    private JTextField _properties = new JTextField(20);
    private JTextArea _comment = new JTextArea(3, 20);

    //
    // ServerDescriptor
    //
    private JTextField _exe = new JTextField(20);
    private JTextField _pwd = new JTextField(20);
    private JTextField _options = new JTextField(20);
    private JTextField _interpreter = new JTextField(20);
    private JTextField _interpreterOptions = new JTextField(20);
    private JTextField _envs = new JTextField(20);
    private JTextField _activation = new JTextField(20);

    //
    // IceBoxDescriptor
    //
    private JLabel _endpointsLabel = new JLabel("IceBox Endpoints");
    private JTextField _endpoints = new JTextField(20);
};
