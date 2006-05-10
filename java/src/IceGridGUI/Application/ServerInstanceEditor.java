// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class ServerInstanceEditor extends AbstractServerEditor
{
    protected void writeDescriptor()
    {
	ServerInstanceDescriptor descriptor = getDescriptor();
	ServerTemplate t = (ServerTemplate)_template.getSelectedItem();
	
	descriptor.template = t.getId();
	descriptor.parameterValues = _parameterValuesMap;

	descriptor.propertySet.references = 
	    (String[])_propertySetsList.toArray(new String[0]);
	descriptor.propertySet.properties = _properties.getProperties();

	((ServerInstance)_target).isIceBox(
	    ((TemplateDescriptor)t.getDescriptor()).descriptor instanceof IceBoxDescriptor);
    }
    
    protected boolean isSimpleUpdate()
    {
	ServerInstanceDescriptor descriptor = getDescriptor();
	ServerTemplate t = (ServerTemplate)_template.getSelectedItem();

	return descriptor.template.equals(t.getId())
	     && descriptor.parameterValues.equals(_parameterValuesMap);
    }

    ServerInstanceEditor(JFrame parentFrame)
    {	
	_template.setToolTipText("Server template");
	_parameterValues.setEditable(false);
	
	//
	// Template
	//
	Action gotoTemplate = new AbstractAction(
	    "", Utils.getIcon("/icons/16x16/goto.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    TreeNode t = (TreeNode)_template.getSelectedItem();
		    if(t != null)
		    {
			t.getRoot().setSelectedNode(t);
		    }
		}
	    };
	gotoTemplate.putValue(Action.SHORT_DESCRIPTION, 
			      "Goto this server template");
	_templateButton = new JButton(gotoTemplate);

	//
	// Parameter values
	//
	_parametersDialog = new ParametersDialog(parentFrame, 
					    "Parameter values",
					    "Value", false, "Use default");
	
	Action openParametersDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    if(_parametersDialog.show(_parameterList, _parameterValuesMap, 
					      getProperties()))
		    {
			updated();
			setParameterValuesField();
			//
			// No need to redisplay details: since it's editable,
			// we're not substituting variables or parameters
			//
		    }
		}
	    };
	openParametersDialog.putValue(Action.SHORT_DESCRIPTION, 
				      "Edit parameters");

	_parametersButton = new JButton(openParametersDialog);


	_propertySets.setEditable(false);
	_properties = new PropertiesField(this);

	_propertySetsDialog = new ListDialog(parentFrame, 
					     "Property Set References", true);

	Action openPropertySetsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _propertySetsDialog.show(
			_propertySetsList, getProperties());
		    if(result != null)
		    {
			updated();
			_propertySetsList = result;
			setPropertySetsField();
		    }
		}
	    };
	openPropertySetsDialog.putValue(Action.SHORT_DESCRIPTION,
					"Edit property set references");
	_propertySetsButton = new JButton(openPropertySetsDialog);
    }

    ServerInstanceDescriptor getDescriptor()
    {
	return (ServerInstanceDescriptor)_target.getDescriptor();
    }

    //
    // From Editor:
    //

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

    protected void appendProperties(DefaultFormBuilder builder)
    { 
	builder.append("Template", _template);
	builder.append(_templateButton);
	builder.nextLine();
	
	builder.append("Parameter values", _parameterValues);
	builder.append(_parametersButton);
	builder.nextLine();
	
	builder.append("Property Sets");
	builder.append(_propertySets, _propertySetsButton);
	builder.nextLine();

	builder.append("Properties");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");

	builder.nextLine();
	builder.append("");

	builder.nextRow(-6);
	JScrollPane scrollPane = new JScrollPane(_properties);
	CellConstraints cc = new CellConstraints();
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
	builder.nextRow(6);
	builder.nextLine();
    }

    void show(ServerInstance server)
    {
	detectUpdates(false);
	_target = server;

	ServerInstanceDescriptor descriptor = getDescriptor();
	Root root = server.getRoot();
	boolean isEditable = !root.getCoordinator().substitute();

	//
	// Need to make control enabled before changing it
	//
	_template.setEnabled(true);

	ServerTemplates serverTemplates = root.getServerTemplates();
	_template.setModel(serverTemplates.createComboBoxModel());

	ServerTemplate t = (ServerTemplate)
	    serverTemplates.findChild(descriptor.template);
	assert t != null;
	_template.setSelectedItem(t);
	
	ListDataListener templateListener = new ListDataListener()
	    {
		public void contentsChanged(ListDataEvent e)
		{
		    updated();
		    
		    ServerTemplate t = 
			(ServerTemplate)_template.getModel().getSelectedItem();
		    
		    TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
		    
		    //
		    // Replace parameters but keep existing values
		    //
		    _parameterList = td.parameters;
		    _parameterValuesMap = makeParameterValues(_parameterValuesMap, 
							      _parameterList);
		    setParameterValuesField();
		}

		public void intervalAdded(ListDataEvent e)
		{}
         
		public void intervalRemoved(ListDataEvent e)
		{}
	    };

	_template.getModel().addListDataListener(templateListener);
	_template.setEnabled(isEditable);
	
	_parameterList =  ((TemplateDescriptor)t.getDescriptor()).parameters;
	_parameterValuesMap = new java.util.HashMap(descriptor.parameterValues);
	setParameterValuesField();
	_parametersButton.setEnabled(isEditable);
	

	_propertySetsList = java.util.Arrays.asList(descriptor.propertySet.references);
	setPropertySetsField();
	_propertySetsButton.setEnabled(isEditable);

	_properties.setProperties(descriptor.propertySet.properties, null, 
				  getDetailResolver(), isEditable);

	_applyButton.setEnabled(server.isEphemeral());
	_discardButton.setEnabled(server.isEphemeral());	  
	detectUpdates(true);
    }

    private void setParameterValuesField()
    {
	final Utils.Resolver resolver = _target.getCoordinator().substitute() ? 
	    ((Node)_target.getParent()).getResolver() : null;
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    String name = (String)obj;
		    String val = (String)_parameterValuesMap.get(name);
		    if(val != null)
		    {
			return name + "=" + Utils.substitute(val, resolver);
		    }
		    else
		    {
			return null;
		    }
		}
	    };
	
	_parameterValues.setText(
	    Utils.stringify(_parameterList, stringifier, ", ", toolTipHolder));
	_parameterValues.setToolTipText(toolTipHolder.value);
    }

    private void setPropertySetsField()
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
	
	_propertySets.setText(
	    Utils.stringify(_propertySetsList, 
			    stringifier, ", ", toolTipHolder));

	String toolTip = "<html>Property Sets";

	if(toolTipHolder.value != null)
	{
	    toolTip += ":<br>" + toolTipHolder.value;
	}
	toolTip += "</html>";
	_propertySets.setToolTipText(toolTip);
    }

    private JComboBox _template = new JComboBox();
    private JButton _templateButton;
    private JTextField _parameterValues = new JTextField(20);

    private java.util.List _parameterList;
    private java.util.Map _parameterValuesMap;

    private ParametersDialog _parametersDialog;
    private JButton _parametersButton;

    private JTextField _propertySets = new JTextField(20);
    private java.util.List _propertySetsList;
    private ListDialog _propertySetsDialog;
    private JButton _propertySetsButton;
 
    private PropertiesField _properties;
}
