// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

import IceGrid.AdapterDynamicInfo;
import IceGrid.ApplicationDescriptor;
import IceGrid.ApplicationUpdateDescriptor;
import IceGrid.DistributionDescriptor;
import IceGrid.Model;
import IceGrid.ServerDynamicInfo;
import IceGrid.ServerState;
import IceGrid.SimpleInternalFrame;
import IceGrid.TemplateDescriptor;


public class Application extends EditableParent
{
    static public ApplicationDescriptor
    copyDescriptor(ApplicationDescriptor ad)
    {
	ApplicationDescriptor copy = (ApplicationDescriptor)ad.clone();
	
	copy.replicatedAdapters = 
	    ReplicatedAdapters.copyDescriptors(copy.replicatedAdapters);
	
	copy.serverTemplates = 
	    ServerTemplates.copyDescriptors(copy.serverTemplates);
	
	copy.serviceTemplates = 
	    ServiceTemplates.copyDescriptors(copy.serviceTemplates);
	
	copy.nodes = Nodes.copyDescriptors(copy.nodes);

	copy.distrib = (DistributionDescriptor)copy.distrib.clone();
	return copy;
    }
    
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	actions[COPY] = true;
	actions[DELETE] = true;

	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof ApplicationDescriptor;
	}
	actions[APPLICATION_REFRESH_INSTALLATION] = true;
	return actions;
    }
    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	    _popup.add(_model.getActions()[APPLICATION_REFRESH_INSTALLATION]);
	}
	return _popup;
    }
    public void copy()
    {
	_model.setClipboard(copyDescriptor(_descriptor));
	_model.getActions()[PASTE].setEnabled(true);
    }
    public void paste()
    {
	_parent.paste();
    }

    public ApplicationUpdateDescriptor createUpdateDescriptor()
    {
	ApplicationUpdateDescriptor update = new ApplicationUpdateDescriptor();
	update.name = _descriptor.name;
	if(isModified())
	{
	    //
	    // Diff description
	    //
	    if(!_descriptor.description.equals(_origDescription))
	    {
		update.description = 
		    new IceGrid.BoxedString(_descriptor.description);
	    }

	    //
	    // Diff variables
	    //
	    update.variables = (java.util.TreeMap)_descriptor.variables.clone();
	    java.util.List removeVariables = new java.util.LinkedList();

	    java.util.Iterator p = _origVariables.entrySet().iterator();
	    while(p.hasNext())
	    {
		java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
		Object key = entry.getKey();
		Object newValue =  update.variables.get(key);
		if(newValue == null)
		{
		    removeVariables.add(key);
		}
		else
		{
		    Object value = entry.getValue();
		    if(newValue.equals(value))
		    {
			update.variables.remove(key);
		    }
		}
	    }
	    update.removeVariables = (String[])removeVariables.toArray(new String[0]);
	}
	else
	{
	    update.variables = new java.util.TreeMap();
	    update.removeVariables = new String[0];
	}

	//
	// Distribution TODO: implement
	//
	//update.distribution = XXX

	//
	// Replicated Adapters
	//
	update.removeReplicatedAdapters = _replicatedAdapters.removedElements();
	update.replicatedAdapters = _replicatedAdapters.getUpdates();
	
	//
	// Server Templates
	//
	update.removeServerTemplates = _serverTemplates.removedElements();
	update.serverTemplates = _serverTemplates.getUpdates();

	//
	// Service Templates
	//
	update.removeServiceTemplates = _serviceTemplates.removedElements();
	update.serviceTemplates =_serviceTemplates.getUpdates();

	//
	// Nodes
	//
	update.removeNodes = _nodes.removedElements();
	update.nodes = _nodes.getUpdates();

	return update;
    }
    
    public void commit()
    {
	super.commit();
	_origVariables = (java.util.Map)_descriptor.variables.clone();
	_origDescription = _descriptor.description;
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	
	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new ApplicationEditor(_model.getMainFrame());
	}
	
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());

	propertiesFrame.validate();
	propertiesFrame.repaint();

    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public Object saveDescriptor()
    {
	ApplicationDescriptor clone = (ApplicationDescriptor)_descriptor.clone();
	clone.distrib = (IceGrid.DistributionDescriptor)clone.distrib.clone();
	return clone;
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	ApplicationDescriptor clone = (ApplicationDescriptor)savedDescriptor;

	_descriptor.variables = clone.variables;
	_descriptor.distrib.icepatch = clone.distrib.icepatch;
	_descriptor.distrib.directories = clone.distrib.directories;
	_descriptor.description = clone.description;
    }
    
    public boolean destroy()
    {
	if(_parent == null)
	{
	    return false;
	}
	Root root = (Root)_parent;
	
	if(_ephemeral)
	{
	    root.removeChild(this, true);
	    return true;
	}
	else if(_model.canUpdate())
	{
	    //
	    // TODO: must save immediately!
	    //
	}
	return false;
    }


    //
    // Builds the application and all its subtrees
    //
    Application(boolean brandNew, ApplicationDescriptor descriptor, Model model)
	throws UpdateFailedException
    {
	super(brandNew, descriptor.name, model);
	_ephemeral = false;
	_descriptor = descriptor;
	_origVariables = (java.util.Map)_descriptor.variables.clone();
	_origDescription = _descriptor.description;


	_replicatedAdapters = new ReplicatedAdapters(_descriptor.replicatedAdapters,
						     _model);
	addChild(_replicatedAdapters);

	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates,
						 _model);	
	addChild(_serviceTemplates);

	_serverTemplates = new ServerTemplates(descriptor.serverTemplates,
					       this);
	addChild(_serverTemplates);
	
	_nodes = new Nodes(_descriptor.nodes, this);
	addChild(_nodes);
    }

    Application(ApplicationDescriptor descriptor, Model model)
    {
	super(false, descriptor.name, model);
	_ephemeral = true;
	_descriptor = descriptor;
    }



    Application(Application o)
    {
	super(o);
	
	_ephemeral = false;
	//
	// We don't deep-copy _descriptor because it's difficult :)
	// So we'll have to be carefull to properly recover the "old" descriptor.
	//
	_descriptor = o._descriptor;
	_origVariables = o._origVariables;
	_origDescription = o._origDescription;
	
	try
	{
	    _replicatedAdapters = new ReplicatedAdapters(o._replicatedAdapters);
	    addChild(_replicatedAdapters);
	    
	    _serviceTemplates = new ServiceTemplates(o._serviceTemplates);
	    addChild(_serviceTemplates);
	    
	    _serverTemplates = new ServerTemplates(o._serverTemplates);
	    addChild(_serverTemplates);
	    
	    _nodes = new Nodes(o._nodes);
	    addChild(_nodes);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
    }

    //
    // Try to apply a major update (the caller must change the descriptor,
    // and restore it if the applyUpdate is unsuccessful
    //
    boolean applyUpdate()
    {
	Application copy = new Application(this);
	
	try
	{
	    update();
	}
	catch(UpdateFailedException e)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		e.toString(),
		"Duplicate id error",
		JOptionPane.ERROR_MESSAGE);
	    
	    _model.getRoot().restore(copy);
	    return false;
	}
	return true;
    }

    //
    // Major update than can't fail
    //
    void applySafeUpdate()
    {
	try
	{
	    update();
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

    void update() throws UpdateFailedException
    {
	_replicatedAdapters.update();
	_serviceTemplates.update();
	_serverTemplates.update();
	_nodes.update();
    }

    //
    // Try to rebuild this application;
    // no-op if it fails!
    //
    void rebuild() throws UpdateFailedException
    {
	_nodes.rebuild();
    }

    //
    // Called when a server-template is deleted, to remove all
    // corresponding instances
    // children.
    //
    void removeServerInstances(String templateId)
    {
	_nodes.removeServerInstances(templateId);
    }

    //
    // Called when a service-template is deleted, to remove all
    // corresponding instances
    //
    void removeServiceInstances(String templateId)
    {
	_nodes.removeServiceInstances(templateId);
	_serverTemplates.removeServiceInstances(templateId);
    }

    void update(ApplicationUpdateDescriptor desc)
	throws UpdateFailedException
    {
	//
	// Description
	//
	if(desc.description != null)
	{
	    _descriptor.description = desc.description.value;
	}

	//
	// Variables
	//
	for(int i = 0; i < desc.removeVariables.length; ++i)
	{
	    _descriptor.variables.remove(desc.removeVariables[i]);
	}
	_descriptor.variables.putAll(desc.variables);


	//
	// Replicated adapters
	//
	for(int i = 0; i < desc.removeReplicatedAdapters.length; ++i)
	{
	    _descriptor.replicatedAdapters.remove(desc.
						  removeReplicatedAdapters[i]);
	}
	_descriptor.replicatedAdapters.addAll(desc.replicatedAdapters);
	_replicatedAdapters.update(desc.replicatedAdapters, 
				   desc.removeReplicatedAdapters);


	//
	// Service templates
	//
	for(int i = 0; i < desc.removeServiceTemplates.length; ++i)
	{
	    _descriptor.serviceTemplates.remove(desc.
						removeServiceTemplates[i]);
	}
	_descriptor.serviceTemplates.putAll(desc.serviceTemplates);
	_serviceTemplates.update(desc.serviceTemplates, 
				 desc.removeServiceTemplates);

	//
	// Server templates
	//
	for(int i = 0; i < desc.removeServerTemplates.length; ++i)
	{
	    _descriptor.serverTemplates.remove(desc.removeServerTemplates[i]);
	}
	_descriptor.serverTemplates.putAll(desc.serverTemplates);
	_serverTemplates.update(desc.serverTemplates, 
				desc.removeServerTemplates);
	
	//
	// Nodes
	//
	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    _descriptor.nodes.remove(desc.removeNodes[i]);
	}
	//
	// Updates also _descriptor.nodes
	//
	_nodes.update(desc.nodes, desc.removeNodes);
    }

    ServerTemplate findServerTemplate(String id)
    {
	return (ServerTemplate)_serverTemplates.findChild(id);
    }

    ServiceTemplate findServiceTemplate(String id)
    {
	return (ServiceTemplate)_serviceTemplates.findChild(id);
    }

    ReplicatedAdapter findReplicatedAdapter(String id)
    {
	return (ReplicatedAdapter)_replicatedAdapters.findChild(id);
    }

    Node findNode(String id)
    {
	return (Node)_nodes.findChild(id);
    }

    java.util.List findServerInstances(String template)
    {
	return _nodes.findServerInstances(template);
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = _serverTemplates.findServiceInstances(template);
	result.addAll(_nodes.findServiceInstances(template));
	return result;
    }


    TemplateDescriptor findServerTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serverTemplates.get(templateName);
    }

    TemplateDescriptor findServiceTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serviceTemplates.get(templateName);
    }
    
    ServerTemplates getServerTemplates()
    {
	return _serverTemplates;
    }
    
    ServiceTemplates getServiceTemplates()
    {
	return _serviceTemplates;
    }

    ReplicatedAdapters getReplicatedAdapters()
    {
	return _replicatedAdapters;
    }

    //
    // Should only be used for reading
    //
    java.util.Map getVariables()
    {
	return _descriptor.variables;
    }

    void nodeUp(String nodeName)
    {
	_nodes.nodeUp(nodeName);
    }

    void nodeDown(String nodeName)
    {
	_nodes.nodeDown(nodeName);
    }
    
    private ApplicationDescriptor _descriptor;
    
    private final boolean _ephemeral;

    //
    // Keeps original version (as deep copies) to be able to build 
    // ApplicationUpdateDescriptor
    //
    private java.util.Map _origVariables;
    private String _origDescription;
   

    //
    // Children
    //
    private ReplicatedAdapters _replicatedAdapters;
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private Nodes _nodes;

    static private ApplicationEditor _editor;
    static private JPopupMenu _popup;
}
