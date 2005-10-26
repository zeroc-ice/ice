// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.Component;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.swing.event.TreeModelEvent;
import javax.swing.tree.TreePath;

import IceGrid.Model;
import IceGrid.Utils;

abstract class CommonBaseI implements CommonBase
{ 
    static protected class PopupMenu extends JPopupMenu
    {
	PopupMenu(final Model model)
	{
	    PopupMenuListener popupListener = new PopupMenuListener()
	    {
		public void popupMenuCanceled(PopupMenuEvent e)
		{
		    model.showActions(model.getSelectedNode());
		}
		  
		public void popupMenuWillBecomeInvisible(PopupMenuEvent e)
		{}
          
		public void popupMenuWillBecomeVisible(PopupMenuEvent e)
		{}
	    };
	    
	    addPopupMenuListener(popupListener);
	}
    }


    public String toString()
    {
	if(isEphemeral())
	{
	    return "*" + _id;
	}
	else
	{
	    return _id;
	}
    }

    public String getId()
    {
	return _id;
    }
    
    public TreePath getPath()
    {
	return _path;
    }
    
    public CommonBase getParent()
    {
	return _parent;
    }
    
    public Editable getEditable()
    {
	if(_parent != null)
	{
	    return _parent.getEditable();
	}
	else
	{
	    return null;
	}
    }

    public boolean isEphemeral()
    {
	return false;
    }

    public boolean destroy()
    {
	// 
	// Destruction not allowed
	//
	return false;
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new Editor(false, false);
	}
	return _editor;
    }

    public Component getTreeCellRendererComponent(
	JTree tree,
	Object value,
	boolean sel,
	boolean expanded,
	boolean leaf,
	int row,
	boolean hasFocus) 
    {
	return null;
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return null;
    }

    public Object getDescriptor()
    {
	return null;
    }

    public Object saveDescriptor()
    {
	assert false;
	return null;
    }

    public void restoreDescriptor(Object d)
    {
	assert false;
    }

    public java.util.List findAllInstances(CommonBase child)
    {
	assert getIndex(child) != -1;

	java.util.List result = new java.util.LinkedList();
	result.add(child);
	return result;
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	return new boolean[ACTION_COUNT];
    }
    public void newAdapter()
    {
	assert false;
    }
    public void newDbEnv()
    {
	assert false;
    }
    public void newNode()
    {
	assert false;
    }
    public void newReplicaGroup()
    {
	assert false;
    }
    public void newServer()
    {
	assert false;
    }
    public void newServerIceBox()
    {
	assert false;
    } 
    public void newServerFromTemplate()
    {
	assert false;
    }
    public void newService()
    {
	assert false;
    }
    public void newServiceFromTemplate()
    {
	assert false;
    }
    public void newTemplateServer()
    {
	assert false;
    }
    public void newTemplateServerIceBox()
    {
	assert false;
    }
    public void newTemplateService()
    {
	assert false;
    }
    public void copy()
    {
	assert false;
    }
    public void paste()
    {
	assert false;
    }
    public void delete()
    {
	TreePath parentPath = getParent().getPath();
	if(destroy())
	{
	    if(parentPath.getPathCount() > 1)
	    {
		_model.setSelectionPath(parentPath);
	    }
	    else
	    {
		// TODO: show splash
	    }
	}
    }
    public void moveUp()
    {
	assert false;
    }
    public void moveDown()
    {
	assert false;
    }
    public void start()
    {
	assert false;
    }
    public void stop()
    {
	assert false;
    }
    public void enable()
    {
	assert false;
    }
    public void disable()
    {
	assert false;
    }
    public void shutdownNode()
    {
	assert false;
    }
    public void applicationInstallDistribution()
    {
	assert false;
    }
    public void serverInstallDistribution()
    {
	assert false;
    }
    
    public JPopupMenu getPopupMenu()
    {
	return null;
    }


    //
    // Fires a nodesChanged event with this node
    //
    void fireNodeChangedEvent(Object source)
    {
	//
	// Bug if I am not attached to the root
	//
	assert _path != null;

	int[] childIndices = new int[1];
	Object[] children = new Object[1];
	children[0] = this;

	TreeModelEvent event;

	if(_parent == null)
	{	 
	    //
	    // I am root
	    //
	    childIndices[0] = 0;
	    Object[] path = null;
	    event = new TreeModelEvent(source, path, childIndices, children); 
	}
	else
	{
	    childIndices[0] = _parent.getIndex(this);
	    event = new TreeModelEvent(source, _parent.getPath(), childIndices, children);
	}
	_model.getTreeModel().fireNodesChangedEvent(event);
    } 
    
    void fireStructureChangedEvent(Object source)
    {
	assert _path != null;

	TreeModelEvent event = new TreeModelEvent(source, _path);
	_model.getTreeModel().fireStructureChangedEvent(event);
    } 

    static String templateLabel(String templateName,
				java.util.List parameters,
				final java.util.Map parameterValues,
				final java.util.Map parameterDefaults)
    {
	String result = templateName + "<";
	
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    String name = (String)obj;
		    String val = (String)parameterValues.get(name);
		    if(val == null)
		    {
			val = (String)parameterDefaults.get(name);
		    }
		    if(val != null)
		    {
			return val;
		    }
		    else
		    {
			return "";
		    }
		}
	    };

	result += Utils.stringify(parameters, stringifier, ", ", null);
	result += ">";
	return result;
    }

    protected CommonBaseI(String id, Model model, boolean root)
    {
	_id = id;
	_model = model;

	if(root)
	{
	    _path = new TreePath(this);
	}
    }

    public Application getApplication()
    {
	if(_path == null || _path.getPathCount() < 2)
	{
	    return null;
	}
	else
	{
	    return (Application)_path.getPath()[1];
	}
    }

    public Model getModel()
    {
	return _model;
    }

    protected void amiSuccess(final String prefix)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _model.getStatusBar().setText(prefix + "done.");
		}
	    });
    }

    protected void amiFailure(String prefix, String title, Ice.UserException e)
    {
	if(e instanceof IceGrid.ServerNotExistException)
	{
	    IceGrid.ServerNotExistException sne =
		(IceGrid.ServerNotExistException)e;

	    amiFailure(prefix, title, 
		       "Server '" + sne.id + 
		       "' was not registered with the IceGrid Registry");
	}
	else if(e instanceof IceGrid.ServerStartException)
	{
	    IceGrid.ServerStartException ste = (IceGrid.ServerStartException)e;
	    amiFailure(prefix, title, "Server '" +
		       ste.id + "' did not start: " + ste.reason);
	}
	else if(e instanceof IceGrid.ApplicationNotExistException)
	{
	    amiFailure(prefix, title, 
		       "This application was not registered with the IceGrid Registry");
	}
	else if(e instanceof IceGrid.PatchException)
	{
	    IceGrid.PatchException pe = (IceGrid.PatchException)e;
	    amiFailure(prefix, title, pe.reason);
	}
	else if(e instanceof IceGrid.NodeNotExistException)
	{
	    IceGrid.NodeNotExistException nnee = 
		(IceGrid.NodeNotExistException)e;

	    amiFailure(prefix, title, 
		       "Node '" + nnee.name + 
		       " 'was not registered with the IceGrid Registry.");
	}
	else if(e instanceof IceGrid.NodeUnreachableException)
	{
	    IceGrid.NodeUnreachableException nue =
		(IceGrid.NodeUnreachableException)e;
	    amiFailure(prefix, title, "Node '" +
		       nue.name + "' is unreachable: " + nue.reason);
	}
	else
	{
	    amiFailure(prefix, title, e.toString());
	}
    }
    
    protected void amiFailure(final String prefix, 
			      final String title, final String message)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {	
		public void run() 
		{
		    failure(prefix, title, message);
		}
	    });
    }

    protected void failure(String prefix, String title, String message)
    {
	_model.getStatusBar().setText(prefix + "failed!");

	JOptionPane.showMessageDialog(
	    _model.getMainFrame(),
	    message,
	    title,
	    JOptionPane.ERROR_MESSAGE);
    }
   
    protected TreePath _path;
    protected Parent _parent;

    //
    // Id (application name, server instance name etc)
    //
    protected String _id;

    //
    // The Model
    //
    protected Model _model; 

    //
    // The default Editor
    //
    static private Editor _editor;
}
