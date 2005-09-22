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
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import javax.swing.tree.TreePath;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.Model;
import IceGrid.PropertyDescriptor;
import IceGrid.Utils;

//
// Base class for all editors
//
abstract class Editor
{   
    abstract void writeDescriptor();
    abstract boolean isSimpleUpdate();
    abstract void append(DefaultFormBuilder builder);

    void postUpdate() {}

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

    protected Editor()
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
    
    protected void setTarget(CommonBase target)
    {
	_target = target;
    }

    CommonBase getTarget()
    {
	return _target;
    }

    

    JComponent getComponent()
    {
	if(_panel == null)
	{
	    //
	    // Build everything using JGoodies's DefaultFormBuilder
	    //
	    FormLayout layout = new FormLayout(
		"right:pref, 3dlu, fill:pref:grow, 3dlu, pref", "");

	    DefaultFormBuilder builder = new DefaultFormBuilder(layout);
	    builder.setBorder(Borders.DLU2_BORDER);
	    builder.setRowGroupingEnabled(true);
	    builder.setLineGapSize(LayoutStyle.getCurrent().getLinePad());

	    append(builder);

	    JScrollPane scrollPane = 
		new JScrollPane(builder.getPanel(),
				JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
				JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	    
	    scrollPane.setBorder(Borders.DIALOG_BORDER);
	    
	    _panel = new JPanel(new BorderLayout());
	    _panel.add(scrollPane, BorderLayout.CENTER);
	    
	    JComponent buttonBar = 
		ButtonBarFactory.buildRightAlignedBar(_applyButton, 
						      _discardButton);
	    buttonBar.setBorder(Borders.DIALOG_BORDER);
	    _panel.add(buttonBar, BorderLayout.SOUTH);
	}
	return _panel;
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
    
    JPanel getPanel()
    {
	return _panel;
    }
    
    
    protected void detectUpdates(boolean val)
    {
	_detectUpdates = val;
    }

    //
    // Update when parent is a SimpleContainer
    //
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
		    SimpleContainer parent = (SimpleContainer)_target.getParent();
		    writeDescriptor();
		    parent.addDescriptor(_target.getDescriptor());
		    _target.destroy(); // just removes the child
		    
		    if(!_target.getApplication().applyUpdate())
		    {
			//
			// Restores old display
			//
			parent = (SimpleContainer)model.findNewNode(parent.getPath());
			parent.removeDescriptor(_target.getDescriptor());
			try
			{
			    parent.addChild(_target, true);
			}
			catch(DuplicateIdException die)
			{
			    assert false;
			}
			_target.setParent(parent);
			model.setSelectionPath(_target.getPath());
			return;
		    }
		    else
		    {
			parent = (SimpleContainer)model.findNewNode(parent.getPath());
			_target = parent.findChildWithDescriptor(_target.getDescriptor());
			model.setSelectionPath(_target.getPath());
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
		    SimpleContainer parent = (SimpleContainer)_target.getParent();
		    writeDescriptor();
		    
		    if(!_target.getApplication().applyUpdate())
		    {
			_target.restoreDescriptor(savedDescriptor);
			
			//
			// Need to find new node!
			//
			_target = model.findNewNode(_target.getPath());
			model.setSelectionPath(_target.getPath());
			//
			//
			// Everything was restored, user must deal with error
			//
			return;
		    }
		    else
		    {
			parent = (SimpleContainer)model.findNewNode(parent.getPath());
			_target = parent.findChildWithDescriptor(_target.getDescriptor());
			model.setSelectionPath(_target.getPath());
		    }
		}
		
		postUpdate();

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
    
    protected void discardUpdate()
    {
	_target.getModel().refreshDisplay();
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

    static java.util.TreeMap makeParameterValues(
	java.util.Map oldParameterValues,
	java.util.List newParameters)
    {
	java.util.TreeMap result = new java.util.TreeMap();

	java.util.Iterator p = newParameters.iterator();
	while(p.hasNext())
	{
	    String name = (String)p.next();
	    String value = (String)oldParameterValues.get(name);
	    if(value == null)
	    {
		value = "";
	    }
	    result.put(name, value);
	}
	return result;
    }

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;
   
    protected CommonBase _target;
    protected JPanel _panel;

    private boolean _detectUpdates = true; 
}
