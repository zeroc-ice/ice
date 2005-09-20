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

import IceGrid.Model;
import IceGrid.PropertyDescriptor;

//
// Base class for all editors
//
abstract class Editor
{   
    abstract void writeDescriptor();
    abstract boolean isSimpleUpdate();
    void postUpdate() {}

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

    protected JPanel buildPanel(JPanel innerPanel)
    {
	JScrollPane scrollPane = 
	    new JScrollPane(innerPanel,
			    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
			    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
	
	scrollPane.setBorder(Borders.DIALOG_BORDER);
	    
	JPanel outerPanel = new JPanel(new BorderLayout());
	outerPanel.add(scrollPane, BorderLayout.CENTER);
	    
	JComponent buttonBar = 
	    ButtonBarFactory.buildRightAlignedBar(_applyButton, 
						  _discardButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);
	outerPanel.add(buttonBar, BorderLayout.SOUTH);
	return outerPanel;
    }

    protected void updated()
    {
	if(_detectUpdates)
	{
	    _applyButton.setEnabled(true);
	    _discardButton.setEnabled(true);
	}
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

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;

    protected CommonBase _target;

    private boolean _detectUpdates = true;   
}
