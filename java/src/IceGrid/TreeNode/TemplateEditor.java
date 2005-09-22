// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class TemplateEditor extends Editor
{
    TemplateEditor()
    {
	_template.getDocument().addDocumentListener(_updateListener);
	_parameters.getDocument().addDocumentListener(_updateListener);
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
	descriptor.parameters = stringToParameters(_parameters.getText());
    }	    
    
    boolean isSimpleUpdate()
    {
	//
	// Not used since we have our own applyUpdate()
	//
	return false;
    }

    void append(DefaultFormBuilder builder)
    { 
	builder.append("Template ID");
	builder.append(_template, 3);
	builder.nextLine();
	
	builder.append("Parameters");
	builder.append(_parameters, 3);
	builder.nextLine();
	
	builder.appendSeparator();
	builder.nextLine();
    }

    void show()
    {
	TemplateDescriptor descriptor = getDescriptor();
	_template.setText(_target.getId());
	_parameters.setText(parametersToString(descriptor.parameters));
    }

    static private java.util.LinkedList stringToParameters(String s)
    {
	java.util.List params = java.util.Arrays.asList(s.split("\\s+"));
	java.util.Collections.sort(params);
	return new java.util.LinkedList(params);
    }

    static private String parametersToString(java.util.List list)
    {
	String result = "";
	java.util.Iterator p = list.iterator();
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

    private JTextField _template = new JTextField(20);
    private JTextField _parameters = new JTextField(20);
}
