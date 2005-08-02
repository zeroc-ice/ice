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
import javax.swing.JPopupMenu;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.event.TreeModelEvent;
import javax.swing.tree.TreePath;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.DefaultComponentFactory;

import IceGrid.TreeModelI;
import IceGrid.Model;

abstract class CommonBaseI implements CommonBase
{ 
    public String toString()
    {
	return _id;
    }

    public String getId()
    {
	return _id;
    }

    public void setParent(CommonBase parent)
    {
	_parent = parent;
	if(_parent == null)
	{
	    _path = null;
	}
	else
	{
	    TreePath parentPath = _parent.getPath();
	    if(parentPath == null)
	    {
		_path = null;
	    }
	    else
	    {
		_path = parentPath.pathByAddingChild(this);
	    }
	}
    }

    public TreePath getPath()
    {
	return _path;
    }
    
    public CommonBase getParent()
    {
	return _parent;
    }


    public JPopupMenu getPopupMenu()
    {
	//
	// Default = no popup menu
	//
	return null;
    }

    public void displayProperties()
    {
	if(_panel == null)
	{
	    JLabel label = DefaultComponentFactory.getInstance().createTitle("This element has no property");
	    _panel = new JPanel();
	    _panel.add(label);
	    _panel.setBorder(Borders.DIALOG_BORDER);
	}

	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties");
	propertiesFrame.setContent(_panel);
	propertiesFrame.validate();
	propertiesFrame.repaint();
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

    static String templateLabel(String templateName, java.util.Collection col)
    {
	String result = templateName + "<";
	
	java.util.Iterator p = col.iterator();
	boolean firstElement = true;
	while(p.hasNext())
	{
	    if(firstElement)
	    {
		firstElement = false;
	    }
	    else
	    {
		result += ", ";
	    }
	    result += (String)p.next();
	}
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

    Model getModel()
    {
	return _model;
    }

    
    protected TreePath _path;
    protected CommonBase _parent;

    //
    // Id (application name, server instance name etc)
    //
    protected String _id;

    //
    // The Model
    //
    protected Model _model; 

    //
    // The default panel
    //
    static protected JPanel _panel;
}
