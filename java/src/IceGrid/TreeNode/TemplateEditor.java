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
import javax.swing.text.*;

import java.util.regex.Pattern;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class TemplateEditor extends Editor
{
    static class ParametersFilter extends DocumentFilter
    {	
	public void insertString(DocumentFilter.FilterBypass fb,
				 int offset,
				 String string,
				 AttributeSet attr)
	    throws BadLocationException
	{
	    if(_pattern.matcher(string).matches())
	    {
		super.insertString(fb, offset, string, attr);
	    }
	}
	
	public void replace(DocumentFilter.FilterBypass fb,
                    int offset,
                    int length,
                    String text,
                    AttributeSet attrs)
             throws BadLocationException
	{
	    if(_pattern.matcher(text).matches())
	    {
		super.replace(fb, offset, length, text, attrs);
	    }
	}
       
	private Pattern _pattern = Pattern.compile("[\\w\\s]*");
    }

    TemplateEditor()
    {
	_template.getDocument().addDocumentListener(_updateListener);
	_parameters.getDocument().addDocumentListener(_updateListener);
	
	AbstractDocument doc = (AbstractDocument)_parameters.getDocument();
	doc.setDocumentFilter(_parametersFilter);
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
	TemplateDescriptor descriptor = getDescriptor();
	return _parameters.getText().equals(descriptor.parameters);
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
	_template.setEditable(_target.isEphemeral());
	_parameters.setText(parametersToString(descriptor.parameters));
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
    private DocumentFilter _parametersFilter = new ParametersFilter();
}
