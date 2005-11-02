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
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import javax.swing.tree.TreePath;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.factories.DefaultComponentFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.Model;
import IceGrid.PropertyDescriptor;
import IceGrid.Utils;

//
// Base class for all editors
//
public class Editor
{     
    public JComponent getProperties()
    {
	if(_propertiesPanel == null && _hasProperties)
	{
	    buildPropertiesPanel();
	}
	return _propertiesPanel;
    }

    public JComponent getCurrentStatus(Ice.StringHolder title)
    {
	if(_currentStatusPanel == null && _hasCurrentStatus)
	{
	    buildCurrentStatusPanel();
	}
	return _currentStatusPanel;
    }

    public JToolBar getCurrentStatusToolBar()
    {
	return null;
    }

    void appendProperties(DefaultFormBuilder builder)
    {}

    void appendCurrentStatus(DefaultFormBuilder builder)
    {}

    public void refreshCurrentStatus()
    {}
    
    protected void applyUpdate()
    {
	assert false;
    }

    private void buildCurrentStatusPanel()
    {
	assert _hasCurrentStatus;

	FormLayout layout = new FormLayout(
	    "right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");
	
	DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	builder.setBorder(Borders.DLU2_BORDER);
	builder.setRowGroupingEnabled(true);
	builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());
	appendCurrentStatus(builder);
	
	JScrollPane scrollPane = 
	    new JScrollPane(builder.getPanel(),
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	
	scrollPane.setBorder(Borders.DIALOG_BORDER);
	
	_currentStatusPanel = new JPanel(new BorderLayout());
	_currentStatusPanel.add(scrollPane, BorderLayout.CENTER);
	_currentStatusPanel.setBorder(Borders.EMPTY_BORDER);
    }
    
    private void buildPropertiesPanel()
    {
	assert _hasProperties;

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
	
	JComponent buttonBar = 
	    ButtonBarFactory.buildRightAlignedBar(_applyButton, 
						  _discardButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);
	_propertiesPanel.add(buttonBar, BorderLayout.SOUTH);
    }
    
    //
    // Used by the sub-editor (when there is one)
    //
    Object getSubDescriptor()
    {
	return null;
    }

    Utils.Resolver getDetailResolver()
    {
	return null;
    }

    protected Editor(boolean hasCurrentStatus, boolean hasProperties)
    {
	_hasCurrentStatus = hasCurrentStatus;
	_hasProperties = hasProperties;

	if(_hasProperties)
	{
	    //
	    // _applyButton
	    //
	    AbstractAction apply = new AbstractAction("Apply")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			applyUpdate();
		    }
		};
	    _applyButton = new JButton(apply);
	    
	    //
	    // _discardButton
	    //
	    AbstractAction discard = new AbstractAction("Discard")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			discardUpdate();
		    }
		};
	    _discardButton = new JButton(discard);
	    
	    _updateListener = new DocumentListener() 
		{
		    public void changedUpdate(DocumentEvent e)
		    {
			updated();
		    }
		    
		    public void insertUpdate(DocumentEvent e)
		    {
			updated();
		    }
		    
		    public void removeUpdate(DocumentEvent e)
		    {
			updated();
		    }
		};
	}
    }
    
    protected void setTarget(CommonBase target)
    {
	_target = target;
    }

    CommonBase getTarget()
    {
	return _target;
    }
    
    void updated()
    {
	if(_detectUpdates)
	{
	    _applyButton.setEnabled(true);
	    _discardButton.setEnabled(true);
	}
    }

    DocumentListener getUpdateListener()
    {
	return _updateListener;
    }
    
    protected void detectUpdates(boolean val)
    {
	_detectUpdates = val;
    }

    protected void discardUpdate()
    {
	if(_target.isEphemeral())
	{
	    _target.delete();
	}
	else
	{
	    _target.getModel().refreshDisplay();
	}
    }
    
    static java.util.Map propertiesToMap(java.util.List props)
    {
	java.util.Map result = new java.util.HashMap(props.size());
	java.util.Iterator p = props.iterator();
	while(p.hasNext())
	{
	    PropertyDescriptor pd = (PropertyDescriptor)p.next();
	    result.put(pd.name, pd.value);
	}
	return result;
    }
    
    static java.util.LinkedList mapToProperties(java.util.Map map)
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

    static java.util.Map makeParameterValues(
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

    private final boolean _hasCurrentStatus;
    private final boolean _hasProperties;

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;
   
    protected CommonBase _target;
    private boolean _detectUpdates = true; 

    protected JPanel _currentStatusPanel;
    protected JPanel _propertiesPanel;
  
}
