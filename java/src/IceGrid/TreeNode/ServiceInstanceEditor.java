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
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;


import javax.swing.tree.TreePath;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Model;
import IceGrid.TableDialog;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class ServiceInstanceEditor extends Editor
{
    ServiceInstanceEditor(JFrame parentFrame)
    {
	_parameterValues.setEditable(false);
	
	//
	// Template
	//
	Action gotoTemplate = new AbstractAction("->")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    CommonBase t = (CommonBase)_template.getSelectedItem();
		    if(t != null)
		    {
			t.getModel().setSelectionPath(t.getPath());
		    }
		}
	    };
	gotoTemplate.putValue(Action.SHORT_DESCRIPTION, 
				       "Goto this template");
	_templateButton = new JButton(gotoTemplate);
	
	
	ListDataListener templateListener = new ListDataListener()
	    {
		public void contentsChanged(ListDataEvent e)
		{
		    updated();
		    
		    ServiceTemplate t = 
			(ServiceTemplate)_template.getModel().getSelectedItem();
		    
		    TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
		    
		    //
		    // Replace parameters but keep existing values
		    //
		    java.util.TreeMap newMap = new java.util.TreeMap();
		    java.util.Iterator p = td.parameters.iterator();
		    while(p.hasNext())
		    {
			String name = (String)p.next();
			newMap.put(name, _parameterValuesMap.get(name));
		    }
		    _parameterValuesMap = newMap;
		    setParameterValuesField();
		}

		public void intervalAdded(ListDataEvent e)
		{}

         
		public void intervalRemoved(ListDataEvent e)
		{}
	    };

	_template.getModel().addListDataListener(templateListener);

	//
	// Parameter values
	//
	_parameterValuesDialog = new TableDialog(parentFrame, 
						 "Parameter values",
						 "Name", 
						 "Value", false);
	
	Action openParameterValuesDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.Map result = 
			_parameterValuesDialog.show(_parameterValuesMap, 
						     _panel);
		    if(result != null)
		    {
			updated();
			_parameterValuesMap = new java.util.TreeMap(result);
			setParameterValuesField();
		    }
		}
	    };
	_parameterValuesButton = new JButton(openParameterValuesDialog);
    }

    ServiceInstanceDescriptor getDescriptor()
    {
	return (ServiceInstanceDescriptor)_target.getDescriptor();
    }

    //
    // From Editor:
    //
    void writeDescriptor()
    {
	ServiceInstanceDescriptor descriptor = getDescriptor();
	descriptor.template = ((ServiceTemplate)_template.getSelectedItem()).getId();
	descriptor.parameterValues = _parameterValuesMap;
    }	    
    
    boolean isSimpleUpdate()
    {
	return false;
    }

    void append(DefaultFormBuilder builder)
    { 
	builder.append("Template", _template);
	builder.append(_templateButton);
	builder.nextLine();
	
	builder.append("Parameter values", _parameterValues);
	builder.append(_parameterValuesButton);
	builder.nextLine();
	
	builder.appendSeparator();
	builder.nextLine();
	
	//
	// TODO: add service fields (read-only)
	//
    }

    void show(Service service)
    {
	detectUpdates(false);
	setTarget(service);
	
	//
	// If it's not a template instance, it's shown using
	// ServiceEditor.show()
	//
	assert getDescriptor().template.length() > 0;


	ServiceInstanceDescriptor descriptor = 
	    (ServiceInstanceDescriptor)service.getDescriptor();
	Model model = service.getModel();

	Utils.Resolver resolver = model.substitute() ? 
	    service.getParentResolver() : null;
	
	boolean isEditable = service.isEditable() && resolver == null;
	
	//
	// Need to make control enabled before changing it
	//
	_template.setEnabled(true);

	ServiceTemplates serviceTemplates =
	    service.getApplication().getServiceTemplates();
	_template.setModel(serviceTemplates.createComboBoxModel());

	ServiceTemplate t = (ServiceTemplate)
	    serviceTemplates.findChild(descriptor.template);
	assert t != null;
	_template.setSelectedItem(t);
	
	_template.setEnabled(isEditable);
	
	_parameterValuesMap = descriptor.parameterValues;
	setParameterValuesField();
	_parameterValues.setEnabled(isEditable);
	
	//
	// TODO: sub editor
	//
	_applyButton.setEnabled(service.isEphemeral());
	_discardButton.setEnabled(service.isEphemeral());	  
	detectUpdates(true);
    }

    void setParameterValuesField()
    {
	Service service = (Service)_target;

	final Utils.Resolver resolver = service.getModel().substitute() ? 
	    service.getParentResolver() : null;
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return Utils.substitute((String)entry.getKey(), resolver) 
			+ "="
			+ Utils.substitute((String)entry.getValue(), resolver);
		}
	    };
	
	_parameterValues.setText(
	    Utils.stringify(_parameterValuesMap.entrySet(), stringifier,
			    ", ", toolTipHolder));
	_parameterValues.setToolTipText(toolTipHolder.value);
    }

    
    private JComboBox _template = new JComboBox();
    private JButton _templateButton;
    private JTextField _parameterValues = new JTextField(20);

    private java.util.TreeMap _parameterValuesMap;
    private TableDialog _parameterValuesDialog;
    private JButton _parameterValuesButton;   
}
