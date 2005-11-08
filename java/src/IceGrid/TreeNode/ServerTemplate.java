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
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.SimpleInternalFrame;

import IceGrid.IceBoxDescriptor;
import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;
import IceGrid.ServerDynamicInfo;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;


class ServerTemplate extends EditableParent
{
    static public TemplateDescriptor
    copyDescriptor(TemplateDescriptor templateDescriptor)
    {
	TemplateDescriptor copy = (TemplateDescriptor)
	    templateDescriptor.clone();

	copy.descriptor = Server.copyDescriptor(
	    (ServerDescriptor)copy.descriptor);
	return copy;
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
	if(_cellRenderer == null)
	{
	    _cellRenderer = new DefaultTreeCellRenderer();
	    _plainIcon =
		Utils.getIcon("/icons/16x16/server_template.png");
	    _iceboxIcon =
		Utils.getIcon("/icons/16x16/icebox_server_template.png");
	}

	if(_templateDescriptor.descriptor instanceof IceBoxDescriptor)
	{
	    if(expanded)
	    {
		_cellRenderer.setOpenIcon(_iceboxIcon);
	    }
	    else
	    {
		_cellRenderer.setClosedIcon(_iceboxIcon);
	    }
	}
	else
	{
	    if(expanded)
	    {
		_cellRenderer.setOpenIcon(_plainIcon);
	    }
	    else
	    {
		_cellRenderer.setClosedIcon(_plainIcon);
	    }
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	if(_parent.getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}

	actions[DELETE] = true;

	actions[NEW_ADAPTER] = (_adapters != null && _services == null);
	actions[NEW_SERVICE] = (_services != null);
	actions[NEW_DBENV] = (_dbEnvs != null);

	return actions;
    }
    public void copy()
    {
	_model.setClipboard(copyDescriptor(_templateDescriptor));
	_model.getActions()[PASTE].setEnabled(true);
    }
    public void paste()
    {
	_parent.paste();
    }
    public void newAdapter()
    {
	_adapters.newAdapter();
    }
    public void newDbEnv()
    {
	_dbEnvs.newDbEnv();
    }
    public void newService()
    {
	_services.newService();
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);

	    JMenuItem item = new JMenuItem(_model.getActions()[NEW_ADAPTER]);
	    item.setText("New adapter");
	    _popup.add(item);

	    item = new JMenuItem(_model.getActions()[NEW_DBENV]);
	    item.setText("New DbEnv");
	    _popup.add(item);

	    item = new JMenuItem(_model.getActions()[NEW_SERVICE]);
	    item.setText("New service");
	    _popup.add(item);
	}
	return _popup;
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new ServerTemplateEditor(_model.getMainFrame());
	}
	_editor.show(this);
	return _editor;
    }
    
    public boolean destroy()
    {
	ServerTemplates serverTemplates = (ServerTemplates)_parent;

	if(serverTemplates != null && _ephemeral)
	{
	    serverTemplates.removeChild(this, true);
	    return true;
	}
	else if(serverTemplates != null && _model.canUpdate())
	{
	    serverTemplates.removeDescriptor(_id);
	    getApplication().removeServerInstances(_id);
	    serverTemplates.removeElement(this, true);
	    return true;
	}
	else
	{
	    return false;
	}
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public java.util.List findAllInstances(CommonBase child)
    {
	java.util.List result = super.findAllInstances(child);
	
	java.util.List serverInstances = 
	    getApplication().findServerInstances(_id);

	java.util.Iterator p = serverInstances.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    result.addAll(server.findChildrenWithType(child.getClass()));
	}

	return result;
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public Object getDescriptor()
    {
	return _templateDescriptor;
    }

    public Object saveDescriptor()
    {
	//
	// Shallow copy
	//
	TemplateDescriptor clone = (TemplateDescriptor)_templateDescriptor.clone();
	clone.descriptor = (ServerDescriptor)_templateDescriptor.descriptor.clone();
	return clone;
    }
    
    public void restoreDescriptor(Object savedDescriptor)
    {
	TemplateDescriptor clone = (TemplateDescriptor)savedDescriptor;
	//
	// Keep the same object
	//
	_templateDescriptor.parameters = clone.parameters;

	Server.shallowRestore((ServerDescriptor)clone.descriptor,
			      (ServerDescriptor)_templateDescriptor.descriptor);
    }

     //
    // Application is needed to lookup service templates
    //
    ServerTemplate(boolean brandNew, String name, TemplateDescriptor descriptor, 
		   Application application)
	throws UpdateFailedException
    {
	super(brandNew, name, application.getModel());
	_ephemeral = false;
	rebuild(descriptor, application);
    }

    ServerTemplate(String name, TemplateDescriptor descriptor,
		   Application application)
    {
	super(false, name, application.getModel());
	_ephemeral = true;
	try
	{
	    rebuild(descriptor, application);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

    java.util.List findServiceInstances(String template)
    {
	if(_services != null)
	{
	    return _services.findServiceInstances(template);
	}
	else
	{
	    return new java.util.LinkedList();
	}
    }
    
    void rebuild(TemplateDescriptor descriptor, Application application)
	throws UpdateFailedException
    {
	_templateDescriptor = descriptor;
	_propertiesHolder = new PropertiesHolder(_templateDescriptor.descriptor);
	clearChildren();

	if(_ephemeral)
	{
	    _adapters = null;
	    _dbEnvs = null;
	    _services = null;
	}
	else
	{ 
	    if(_templateDescriptor.descriptor instanceof IceBoxDescriptor)
	    {
		IceBoxDescriptor iceBoxDescriptor = 
		    (IceBoxDescriptor)_templateDescriptor.descriptor;
		
		_services = new Services(iceBoxDescriptor.services, true, null, 
					 application);
		addChild(_services);
		
		assert _templateDescriptor.descriptor.dbEnvs.size() == 0;
		_dbEnvs = null;
	    }
	    else
	    {
		_services = null;
		
		_dbEnvs = new DbEnvs(_templateDescriptor.descriptor.dbEnvs, true,
				     null, _model);
		addChild(_dbEnvs);
	    }
	    
	    _adapters = new Adapters(_templateDescriptor.descriptor.adapters, true, 
				     _services != null, null, _model);
	    addChild(_adapters);
	}
    }

    void rebuild()
	throws UpdateFailedException
    {
	rebuild(_templateDescriptor, getApplication());
    }

    void removeServiceInstances(String template)
    {
	if(_services != null)
	{
	    _services.removeServiceInstances(template);
	}
    }

    private TemplateDescriptor _templateDescriptor;
 
    private Services _services;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;
    private final boolean _ephemeral;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _plainIcon;
    static private Icon _iceboxIcon;
    static private ServerTemplateEditor _editor;
    static private JPopupMenu _popup;
}
