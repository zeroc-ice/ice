// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.Component;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class Service extends Communicator implements Cloneable, TemplateInstance
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
	copy.adapters = Adapter.copyDescriptors(copy.adapters);
	copy.dbEnvs = DbEnv.copyDescriptors(copy.dbEnvs);
	
	//
	// Update to properties is not atomic because of Adapter endpoints
	// (and possibly other properties set through a PropertiesHolder)
	//
	//
	// TODO: BENOIT: Add support for property set.
	//
	copy.propertySet = new PropertySetDescriptor(new String[0], 
						     (java.util.LinkedList)copy.propertySet.properties.clone());
	return copy;
    }
    
    static public java.util.LinkedList
    copyDescriptors(java.util.LinkedList descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(copyDescriptor(
			 (ServiceInstanceDescriptor)p.next()));
	}
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
	    _cellRenderer.setOpenIcon(
		Utils.getIcon("/icons/16x16/service.png"));

	    _cellRenderer.setClosedIcon(
		Utils.getIcon("/icons/16x16/service.png"));
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
	
	if(((TreeNode)_parent).getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}

	if(_isEditable)
	{
	    actions[DELETE] = true;

	    if(_instanceDescriptor.template.equals(""))
	    {
		actions[NEW_ADAPTER] = !_ephemeral;
		actions[NEW_DBENV] = !_ephemeral;
	    }
	}
	
	if(_parent instanceof Server && !_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;
	}
	
	actions[MOVE_UP] = canMove(true);
	actions[MOVE_DOWN] = canMove(false);
	return actions;
    }
    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = getCoordinator().getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_ADAPTER));
	    _popup.add(actions.get(NEW_DBENV));
	    _popup.addSeparator();
	    _popup.add(actions.get(MOVE_UP));
	    _popup.add(actions.get(MOVE_DOWN));
	}
	actions.setTarget(this);
	return _popup;
    }
    public void copy()
    {
	getCoordinator().setClipboard(copyDescriptor(_instanceDescriptor));
	getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }
    
    public void moveUp()
    {
	move(true);
    }
    public void moveDown()
    {
	move(false);
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
	    //
	    // TODO: BENOIT: Add support for property set.
	    //
	    _instanceDescriptor.descriptor.propertySet = 
		new PropertySetDescriptor(new String[0], sd.descriptor.propertySet.properties);
	    _instanceDescriptor.descriptor.description = sd.descriptor.description;
	    
	    _instanceDescriptor.descriptor.name = sd.descriptor.name;
	    _instanceDescriptor.descriptor.entry = sd.descriptor.entry;
	}
    }

    public void destroy()
    {
	((Communicator)_parent).getServices().destroyChild(this);
    }

    public Editor getEditor()
    {
	if(_instanceDescriptor.template.length() > 0)
	{
	    if(_instanceEditor == null)
	    {
		_instanceEditor = (ServiceInstanceEditor)getRoot().
		    getEditor(ServiceInstanceEditor.class, this);
	    }
	    _instanceEditor.show(this);
	    return _instanceEditor;
	}
	else
	{
	    if(_editor == null)
	    {
		_editor = (ServiceEditor)getRoot().getEditor(ServiceEditor.class, this);
	    }
	    _editor.show(this);
	    return _editor;
	}
    }

    protected Editor createEditor()
    {
	if(_instanceDescriptor.template.length() > 0)
	{
	    return new ServiceInstanceEditor(getCoordinator().getMainFrame());
	}
	else
	{
	    return new ServiceEditor(getCoordinator().getMainFrame());
	}
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
    
    Editable getEnclosingEditable()
    {
	return ((Communicator)_parent).getEnclosingEditable();
    }

    private boolean canMove(boolean up)
    {
	if(_ephemeral)
	{
	    return false;
	}
	else
	{
	    return ((Communicator)_parent).getServices().canMove(this, up);
	}
    }

    private void move(boolean up)
    {
	assert canMove(up);
	((Communicator)_parent).getServices().move(this, up);
    }
    
    
    java.util.List findInstances(boolean includeTemplate)
    {
	java.util.List result = new java.util.LinkedList();

	//
	// First find all instances of the enclosing Communicator, including
	// the ServerTemplate itself (if that's my parent)
	//
	java.util.List communicatorList = ((Communicator)_parent).findInstances(true);
	
	java.util.Iterator p = communicatorList.iterator();
	while(p.hasNext())
	{
	    Services services = ((Communicator)p.next()).getServices();
	    Service service = (Service)services.findChildWithDescriptor(_instanceDescriptor);
	    assert service != null;
	    result.add(service);
	}
	return result;
    }
    
    static private class Backup
    {
	java.util.Map parameterValues;
	Service clone;
    }

    public Object rebuild(java.util.List editables) 
	throws UpdateFailedException
    {
	Backup backup = new Backup();

	//
	// Fix-up _instanceDescriptor if necessary
	//
	if(_instanceDescriptor.template.length() > 0)
	{
	    TemplateDescriptor templateDescriptor 
		= getRoot().findServiceTemplateDescriptor(_instanceDescriptor.template);

	    java.util.Set parameters = new java.util.HashSet(templateDescriptor.parameters);
	    if(!parameters.equals(_instanceDescriptor.parameterValues.keySet()))
	    {
		backup.parameterValues = _instanceDescriptor.parameterValues;
		_instanceDescriptor.parameterValues = EditorBase.makeParameterValues(
		    _instanceDescriptor.parameterValues, templateDescriptor.parameters);
		editables.add(getEnclosingEditable());
	    }
	}

	Communicator communicator = (Communicator)_parent;
	Services services = communicator.getServices();
	Service newService = null;

	try
	{
	    newService = (Service)services.createChild(_instanceDescriptor);
	}
	catch(UpdateFailedException e)
	{
	    if(backup.parameterValues != null)
	    {
		_instanceDescriptor.parameterValues = backup.parameterValues;
	    }
	    throw e;
	}

	try
	{
	    backup.clone = (Service)clone();
	}
	catch(CloneNotSupportedException e)
	{
	    assert false;
	}

	reset(newService);

	if(backup.parameterValues != null)
	{
	    editables.add(getEnclosingEditable());
	}
	getRoot().getTreeModel().nodeChanged(this);
	return backup;
    }

    public void restore(Object backupObj)
    {
	Backup backup = (Backup)backupObj;

	if(backup.parameterValues != null)
	{
	    _instanceDescriptor.parameterValues = backup.parameterValues;
	}
	
	reset(backup.clone);
	getRoot().getTreeModel().nodeChanged(this);
    }

    private void reset(Service from)
    {
	_id = from._id;
	assert _parent == from._parent;

	_adapters = from._adapters;
	_dbEnvs = from._dbEnvs;
	_services = from._services;
	_childListArray = from._childListArray;
	
	assert _instanceDescriptor == from._instanceDescriptor;
	_serviceDescriptor = from._serviceDescriptor;

	_displayString = from._displayString;
	_isEditable = from._isEditable;
	_resolver = from._resolver;
    }

    Service(Communicator parent,
	    String name,
	    String displayString,
	    ServiceInstanceDescriptor instanceDescriptor, 
	    ServiceDescriptor serviceDescriptor,
	    Utils.Resolver resolver)
	throws UpdateFailedException
    {
	super(parent, name);
	_displayString = displayString;
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = serviceDescriptor;
	_ephemeral = false;
	_resolver = resolver;

	_isEditable = _instanceDescriptor.template.length() == 0 
	    && ((TreeNode)_parent).isEditable();

	_adapters.init(_serviceDescriptor.adapters);
	_dbEnvs.init(_serviceDescriptor.dbEnvs);
    }

    //
    // New temporary object
    //
    Service(Communicator parent, String name,
	    ServiceInstanceDescriptor instanceDescriptor)
    {
	super(parent, name);
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = instanceDescriptor.descriptor;
	_ephemeral = true;
	_isEditable = true;
    }

    static java.util.List createAttributes(ServiceDescriptor descriptor)
    {
	java.util.List attributes = new java.util.LinkedList();
	attributes.add(createAttribute("name", descriptor.name));
	attributes.add(createAttribute("entry", descriptor.entry));
	return attributes;
    }

    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    if(_instanceDescriptor.template.length() > 0)
	    {
		TemplateDescriptor templateDescriptor 
		    = getRoot().findServiceTemplateDescriptor(_instanceDescriptor.template);

		java.util.LinkedList attributes = parameterValuesToAttributes(
		    _instanceDescriptor.parameterValues, templateDescriptor.parameters);
		attributes.addFirst(createAttribute("template", _instanceDescriptor.template));
		
		writer.writeElement("service-instance", attributes);
	    }
	    else
	    {
		writer.writeStartTag("service", createAttributes(_serviceDescriptor));

		if(_serviceDescriptor.description.length() > 0)
		{
		    writer.writeElement("description", _serviceDescriptor.description);
		}
		//
		// TODO: BENOIT: Add support for property set.
		//
		writeProperties(writer, _serviceDescriptor.propertySet.properties);
		_adapters.write(writer);
		_dbEnvs.write(writer);
		writer.writeEndTag("service");
	    }
	}
    }

    ServiceDescriptor getServiceDescriptor()
    {
	return _serviceDescriptor;
    }

    CommunicatorDescriptor getCommunicatorDescriptor()
    {
	return _serviceDescriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    boolean isEditable()
    {
	return _isEditable;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;

    private String _displayString;
    private final boolean _ephemeral;
    private boolean _isEditable;

    private Utils.Resolver _resolver;
    private ServiceEditor _editor;
    private ServiceInstanceEditor _instanceEditor;

    static private DefaultTreeCellRenderer _cellRenderer;  
    static private JPopupMenu _popup;
}
