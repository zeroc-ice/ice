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
import javax.swing.JTextField;


import java.util.regex.Pattern;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.ParametersDialog;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class TemplateEditor extends Editor
{
    TemplateEditor(JFrame parentFrame)
    {
	_template.getDocument().addDocumentListener(_updateListener);
	_parameters.setEditable(false);

	//
	// Parameters
	//
	_parametersDialog = new ParametersDialog(parentFrame, 
						 "Parameters",
						 "Default value", true, "No default");
	
	Action openParametersDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    if(_parametersDialog.show(_parameterList, _parameterValuesMap, 
					      _panel))
		    {
			updated();
			setParametersField();
			//
			// No need to redisplay details: since it's editable,
			// we're not substituting variables or parameters
			//
		    }
		}
	    };
	_parametersButton = new JButton(openParametersDialog);
    }

    TemplateDescriptor getDescriptor()
    {
	return (TemplateDescriptor)_target.getDescriptor();
    }
    
    Object getSubDescriptor()
    {
	return getDescriptor().descriptor;
    }

    void writeDescriptor()
    {
	TemplateDescriptor descriptor = getDescriptor();
	descriptor.parameters = _parameterList;
	descriptor.parameterDefaults = _parameterValuesMap;
    }	    
    
    boolean isSimpleUpdate()
    {
	TemplateDescriptor descriptor = getDescriptor();
	return descriptor.parameters.equals(_parameterList)
	    && descriptor.parameterDefaults.equals(_parameterValuesMap);
    }

    void append(DefaultFormBuilder builder)
    { 
	builder.append("Template ID");
	builder.append(_template, 3);
	builder.nextLine();
	
	builder.append("Parameters", _parameters);
	builder.append(_parametersButton);
	builder.nextLine();
	
	builder.appendSeparator();
	builder.nextLine();
    }

    void show()
    {
	TemplateDescriptor descriptor = getDescriptor();
	_template.setText(_target.getId());
	_template.setEditable(_target.isEphemeral());

	_parameterList = new java.util.LinkedList(descriptor.parameters);
	_parameterValuesMap = new java.util.HashMap(descriptor.parameterDefaults);
	setParametersField();
    }

    protected void applyUpdate()
    {
	Model model = _target.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(_target.isEphemeral())
		{
		    writeDescriptor();
		    TemplateDescriptor descriptor = getDescriptor();
		    Templates parent = (Templates)_target.getParent();
		    _target.destroy(); // just removes the child

		    if(!parent.tryAdd(_template.getText(), descriptor))
		    {
			try
			{
			    parent.addChild(_target, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			return;
		    }
		    else
		    {
			_target = parent.findChildWithDescriptor(descriptor);
			model.setSelectionPath(_target.getPath());
			model.showActions(_target);
			_template.setEditable(false);
		    }
		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = _target.saveDescriptor();
		    Templates parent = (Templates)_target.getParent();
		    writeDescriptor();
		    
		    if(!parent.tryUpdate(_target))
		    {
			_target.restoreDescriptor(savedDescriptor);
			
			//
			// Everything was restored, user must deal with error
			//
			return;
		    }
		    else
		    {
			_target = parent.findChildWithDescriptor(getDescriptor());
			model.setSelectionPath(_target.getPath());
			model.showActions(_target);
		    }
		}

		_target.getEditable().markModified();
		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	}
    }

    private void setParametersField()
    {	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    String name = (String)obj;
		    String val = (String)_parameterValuesMap.get(name);
		    if(val != null)
		    {
			return name + "=" + val;
		    }
		    else
		    {
			return name;
		    }
		}
	    };
	
	_parameters.setText(
	    Utils.stringify(_parameterList, stringifier, ", ", toolTipHolder));
	_parameters.setToolTipText(toolTipHolder.value);
    }

    private JTextField _template = new JTextField(20);
    private JTextField _parameters = new JTextField(20);
    
    private java.util.LinkedList _parameterList;
    private java.util.Map _parameterValuesMap;

    private ParametersDialog _parametersDialog;
    private JButton _parametersButton;   
}
