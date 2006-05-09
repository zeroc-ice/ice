// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.BorderLayout;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;

//
// Base class for all editors
//
public abstract class EditorBase
{     
    public JComponent getProperties()
    {
	if(_propertiesPanel == null)
	{
	    buildPropertiesPanel();
	}
	return _propertiesPanel;
    }
    
    public JToolBar getToolBar()
    {
	return null;
    }

    
   
    static public java.util.SortedMap propertiesToMap(java.util.List props, 
						      Utils.Resolver resolver)
    {
	java.util.SortedMap result = new java.util.TreeMap();
	java.util.Iterator p = props.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    String name = (String)pd.name;
	    String val = (String)pd.value;
	    if(resolver != null)
	    {
		name = resolver.substitute(pd.name);
		val = resolver.substitute(pd.value);
	    }
	    result.put(name, val);
	}
	return result;
    }

    static public java.util.Vector mapToVector(java.util.Map map)
    {
	java.util.Vector result = new java.util.Vector(map.size());
	java.util.Iterator p = map.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    java.util.Vector row = new java.util.Vector(2);
	    row.add(entry.getKey());
	    row.add(entry.getValue());
	    result.add(row);
	}
	return result;
    }
 
    static public java.util.LinkedList mapToProperties(java.util.Map map)
    {
	java.util.LinkedList result = new java.util.LinkedList();
	java.util.Iterator p = map.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    String value = (String)entry.getValue();
	    result.add(new PropertyDescriptor(name, value));
	}
	return result;
    }

    static public java.util.Map makeParameterValues(
	java.util.Map oldParameterValues,
	java.util.List newParameters)
    {
	java.util.Map result = new java.util.HashMap();

	java.util.Iterator p = newParameters.iterator();
	while(p.hasNext())
	{
	    Object name =  p.next();
	    Object value = oldParameterValues.get(name);
	    if(value != null)
	    {
		result.put(name, value);
	    } 
	}
	return result;
    }
    
    protected abstract void appendProperties(DefaultFormBuilder builder);

    protected void buildPropertiesPanel()
    {
	FormLayout layout = new FormLayout(
	    "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
	
	DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	builder.setBorder(Borders.DLU2_BORDER);
	builder.setRowGroupingEnabled(true);
	builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
	
	appendProperties(builder);
	
	JScrollPane scrollPane = 
	    new JScrollPane(builder.getPanel(),
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	
	scrollPane.setBorder(Borders.DIALOG_BORDER);
	
	_propertiesPanel = new JPanel(new BorderLayout());
	_propertiesPanel.add(scrollPane, BorderLayout.CENTER);
	_propertiesPanel.setBorder(Borders.EMPTY_BORDER);
    }
    
    protected JPanel _propertiesPanel;  
}
