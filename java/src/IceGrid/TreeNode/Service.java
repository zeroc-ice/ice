// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.SimpleInternalFrame;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Service extends Parent
{
    static public ServiceInstanceDescriptor
    copyDescriptor(ServiceInstanceDescriptor instanceDescriptor)
    {
	ServiceInstanceDescriptor copy = (ServiceInstanceDescriptor)
	    instanceDescriptor.clone();
	
	if(copy.descriptor != null)
	{
	    copy.descriptor = copyDescriptor((ServiceDescriptor)copy.descriptor);
	}
	return copy;
    }
    
    static public ServiceDescriptor
    copyDescriptor(ServiceDescriptor sd)
    {
	ServiceDescriptor copy = (ServiceDescriptor)sd.clone();
	copy.adapters = Adapters.copyDescriptors(copy.adapters);
	copy.dbEnvs = DbEnvs.copyDescriptors(copy.dbEnvs);
	
	//
	// Update to properties is not atomic because of Adapter endpoints
	// (and possibly other properties set through a PropertiesHolder)
	//
	copy.properties = (java.util.LinkedList)copy.properties.clone();
	return copy;
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
	if(isEditable())
	{
	    actions[DELETE] = true;

	    if(_instanceDescriptor.template.equals(""))
	    {
		actions[NEW_ADAPTER] = (_adapters != null);
		actions[NEW_DBENV] = (_dbEnvs != null);
	    }
	}
	
	if(_resolver != null && !_ephemeral)
	{
	    actions[SUBSTITUTE_VARS] = true;
	}
	
	actions[MOVE_UP] = canMoveUp();
	actions[MOVE_DOWN] = canMoveDown();
	return actions;
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
	    
	    _popup.addSeparator();
	    _popup.add(_model.getActions()[MOVE_UP]);
	    _popup.add(_model.getActions()[MOVE_DOWN]);
	}
	return _popup;
    }
    public void copy()
    {
	_model.setClipboard(copyDescriptor(_instanceDescriptor));
	_model.getActions()[PASTE].setEnabled(true);
    }
    public void paste()
    {
	_parent.paste();
    }
    public void moveUp()
    {
	assert canMoveUp();
	((Services)_parent).move(this, true);
    }
    public void moveDown()
    {
	assert canMoveDown();
	((Services)_parent).move(this, false);
    }
    public void newAdapter()
    {
	_adapters.newAdapter();
    }
    public void newDbEnv()
    {
	_dbEnvs.newDbEnv();
    }

    public Object getDescriptor()
    {
	return _instanceDescriptor;
    }

    public Object saveDescriptor()
    {
	//
	// Must be a shallow copy
	//
	ServiceInstanceDescriptor saved = 
	    (ServiceInstanceDescriptor)_instanceDescriptor.clone();
	
	if(saved.descriptor != null)
	{
	    saved.descriptor = (ServiceDescriptor)saved.descriptor.clone();
	}
	return saved;
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	ServiceInstanceDescriptor sd = (ServiceInstanceDescriptor)savedDescriptor;
	_instanceDescriptor.template = sd.template;
	_instanceDescriptor.parameterValues = sd.parameterValues;
	if(_instanceDescriptor.descriptor != null)
	{
	    _instanceDescriptor.descriptor.properties = sd.descriptor.properties;
	    _instanceDescriptor.descriptor.description = sd.descriptor.description;
	    
	    _instanceDescriptor.descriptor.name = sd.descriptor.name;
	    _instanceDescriptor.descriptor.entry = sd.descriptor.entry;
	}
    }

    public boolean destroy()
    {
	return _parent == null ? false : 
	    ((ListParent)_parent).destroyChild(this);
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties for " + _id);
	
	if(_instanceDescriptor.template.length() > 0)
	{
	    if(_instanceEditor == null)
	    {
		_instanceEditor = new ServiceInstanceEditor(_model.getMainFrame());
	    }
	    _instanceEditor.show(this);
	    propertiesFrame.setContent(_instanceEditor.getComponent());
	}
	else
	{
	    if(_editor == null)
	    {
		_editor = new ServiceEditor(_model.getMainFrame());
	    }
	    _editor.show(this);
	    propertiesFrame.setContent(_editor.getComponent());
	}
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }

    public String toString()
    {
	if(_displayString != null)
	{
	    return _displayString;
	}
	else
	{
	    return super.toString();
	}
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

  
    boolean canMoveUp()
    {
	if(_ephemeral)
	{
	    return false;
	}
	else
	{
	    return ((Services)_parent).canMove(this, true);
	}
    }

    boolean canMoveDown()
    {
	if(_ephemeral)
	{
	    return false;
	}
	else
	{
	    return ((Services)_parent).canMove(this, false);
	}
    }


    //
    // child == _adapters or _dbEnvs
    //
    public java.util.List findAllInstances(CommonBase child)
    {
	//
	// Find all instances of this service (in server instances)
	// and return the list of their adapters or dbEnvs
	//
	assert getIndex(child) != -1;

	java.util.List result = new java.util.LinkedList();

	//
	// First find all instances of the enclosing Services
	//
	java.util.List servicesList = _parent.getParent().findAllInstances(_parent);
	
	java.util.Iterator p = servicesList.iterator();
	while(p.hasNext())
	{
	    Services services = (Services)p.next();
	    Service service = (Service)services.findChildWithDescriptor(_instanceDescriptor);
	    assert service != null;
	    result.addAll(service.findChildrenWithType(child.getClass()));
	}
	return result;
    }


    Service(String name,
	    String displayString,
	    ServiceInstanceDescriptor instanceDescriptor, 
	    ServiceDescriptor serviceDescriptor,
	    boolean isEditable,
	    Utils.Resolver resolver,
	    Model model)
	throws UpdateFailedException
    {
	super(name, model);
	_displayString = displayString;
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = serviceDescriptor;
	_propertiesHolder = new PropertiesHolder(serviceDescriptor);
	_ephemeral = false;

	_resolver = resolver;

	boolean areChildrenEditable = _instanceDescriptor.template.length() == 0 
	    && isEditable;

	try
	{
	    _adapters = new Adapters(serviceDescriptor.adapters, 
				     areChildrenEditable, false, resolver, 
				     _model);
	    addChild(_adapters);
	    
	    _dbEnvs = new DbEnvs(serviceDescriptor.dbEnvs, 
				 areChildrenEditable, resolver, _model);
	    addChild(_dbEnvs);
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}
    }

    //
    // New temporary object
    //
    Service(String name,
	    ServiceInstanceDescriptor instanceDescriptor,
	    Model model)
    {
	super(name, model);
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = instanceDescriptor.descriptor;
	_ephemeral = true;
    }


    ServiceDescriptor getServiceDescriptor()
    {
	return _serviceDescriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    Utils.Resolver getParentResolver()
    {
	return ((Services)_parent).getResolver();
    }

    boolean isEditable()
    {
	if(_parent == null)
	{
	    return false;
	}
	else
	{
	    return ((Services)_parent).isEditable();
	}
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;

    private String _displayString;
    private final boolean _ephemeral;
    private Utils.Resolver _resolver;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;

    static private ServiceEditor _editor;
    static private ServiceInstanceEditor _instanceEditor;
    static private JPopupMenu _popup;
}
