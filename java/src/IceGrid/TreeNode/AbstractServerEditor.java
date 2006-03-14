// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;

//
// Base class for ServerEditor and ServerInstanceEditor
//

abstract class AbstractServerEditor extends Editor
{
    abstract protected void writeDescriptor();
    abstract protected boolean isSimpleUpdate();

    private class ToolBar extends JToolBar
    {
	private ToolBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.SINGLE);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
	    setFloatable(false);
	    putClientProperty("JToolBar.isRollover", Boolean.TRUE);
	    
	    add(_model.getActions()[CommonBase.START]);
	    add(_model.getActions()[CommonBase.STOP]);
	    addSeparator();
	    add(_model.getActions()[CommonBase.ENABLE]);
	    add(_model.getActions()[CommonBase.DISABLE]);
	}
    }


    public JToolBar getCurrentStatusToolBar()
    {
	if(_toolBar == null)
	{
	    _toolBar = new ToolBar();
	}
	return _toolBar;
    }

    public void refreshCurrentStatus()
    {
	Server server = (Server)_target;
	
	ServerState state = server.getState();
	if(state == null)
	{
	    _currentState.setText("Unknown");
	    _currentPid.setText("");
	}
	else
	{
	    _currentState.setText(state.toString());
	    int pid = server.getPid();
	    if(pid == 0)
	    {
		_currentPid.setText("");
	    }
	    else
	    {
		_currentPid.setText(Integer.toString(pid));
	    }
	}
    }


    protected void applyUpdate()
    {
	Server server = (Server)_target;
	Model model = server.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(server.isEphemeral())
		{
		    Node node = (Node)server.getParent();
		    writeDescriptor();
		    ServerInstanceDescriptor instanceDescriptor =
			server.getInstanceDescriptor();
		    ServerDescriptor serverDescriptor = 
			server.getServerDescriptor();

		    server.destroy(); // just removes the child
		    
		    try
		    {
			node.tryAdd(instanceDescriptor, serverDescriptor, true);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Add back ephemeral child
			//
			try
			{
			    node.addChild(server, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(server.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }

		    //
		    // Success
		    //
		    if(instanceDescriptor != null)
		    {
			_target = node.findChildWithDescriptor(instanceDescriptor);
		    }
		    else
		    {
			_target = node.findChildWithDescriptor(serverDescriptor);
		    }
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		    //
		    // If this new server has a known state, display it
		    //
		    ((Server)_target).fireNodeChangedEvent(this);

		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		    _target.getEditable().markModified();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = server.saveDescriptor();
		    Node node = (Node)server.getParent();
		    writeDescriptor();
		    
		    ServerInstanceDescriptor instanceDescriptor =
			server.getInstanceDescriptor();
		    ServerDescriptor serverDescriptor = 
			server.getServerDescriptor();

		    node.removeChild(server, true);
		    
		    try
		    {
			node.tryAdd(instanceDescriptor, serverDescriptor, false);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Restore all
			//	
			try
			{
			    node.addChild(server, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			server.restoreDescriptor(savedDescriptor);
			model.setSelectionPath(server.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }
		    
		    //
		    // Success
		    //
		    node.removeElement(server.getId()); // replaced by brand new Server
		    
		    if(instanceDescriptor != null)
		    {
			_target = node.findChildWithDescriptor(instanceDescriptor);
		    }
		    else
		    {
			_target = node.findChildWithDescriptor(serverDescriptor);
		    }
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);

		    //
		    // If this server has a known state, display it
		    //
		    ((Server)_target).fireNodeChangedEvent(this);

		}

		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	}
    }

 
    void appendCurrentStatus(DefaultFormBuilder builder)
    {
	builder.append("State");
	builder.append(_currentState, 3);
	builder.nextLine();

	builder.append("Process ID");
	builder.append(_currentPid, 3);
	builder.nextLine();
    }
  
    protected AbstractServerEditor(Model model)
    {
	super(true, true);
	_model = model;
	
	_currentState.setEditable(false);
	_currentPid.setEditable(false);
    }

    private JTextField _currentState = new JTextField(20);
    private JTextField _currentPid = new JTextField(20);

    private JToolBar _toolBar;
    private Model _model;
}
