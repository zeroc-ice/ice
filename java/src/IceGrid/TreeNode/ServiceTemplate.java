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
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.SimpleInternalFrame;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class ServiceTemplate extends EditableParent
{
    static public TemplateDescriptor
    copyDescriptor(TemplateDescriptor templateDescriptor)
    {
	TemplateDescriptor copy = (TemplateDescriptor)
	    templateDescriptor.clone();

	copy.descriptor = Service.copyDescriptor( 
	    (ServiceDescriptor)copy.descriptor);
	
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
		Utils.getIcon("/icons/16x16/service_template.png"));
	    _cellRenderer.setClosedIcon(
		Utils.getIcon("/icons/16x16/service_template.png"));
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

	actions[NEW_ADAPTER] = (_adapters != null);
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
	}
	return _popup;
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new ServiceTemplateEditor(_model.getMainFrame());
	}
	_editor.show(this);
	return _editor;
    }

    ServiceTemplate(boolean brandNew, String name, 
		    TemplateDescriptor descriptor, Model model)
	throws UpdateFailedException
    {
	super(brandNew, name, model);
	_ephemeral = false;
	rebuild(descriptor);
    }
    
    ServiceTemplate(String name, TemplateDescriptor descriptor, Model model)
    {
	super(false, name, model);
	_ephemeral = true;
	try
	{
	    rebuild(descriptor);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

    void rebuild(TemplateDescriptor descriptor)
	throws UpdateFailedException
    {
	_templateDescriptor = descriptor;
	_propertiesHolder = new PropertiesHolder(_templateDescriptor.descriptor);
	
	clearChildren();
	if(_ephemeral)
	{
	    _adapters = null;
	    _dbEnvs = null;
	}
	else
	{
	    _adapters = new Adapters(_templateDescriptor.descriptor.adapters, true, 
				     false, null, _model);
	    addChild(_adapters);
	    
	    _dbEnvs = new DbEnvs(_templateDescriptor.descriptor.dbEnvs, true,
				 null, _model);
	    addChild(_dbEnvs);
	}
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public Object getDescriptor()
    {
	return _templateDescriptor;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }
    
    public boolean destroy()
    {
	ServiceTemplates serviceTemplates = (ServiceTemplates)_parent;

	if(serviceTemplates != null && _ephemeral)
	{
	    serviceTemplates.removeChild(this, true);
	    return true;
	}
	else if(serviceTemplates != null && _model.canUpdate())
	{
	    serviceTemplates.removeDescriptor(_id);
	    getApplication().removeServiceInstances(_id);
	    serviceTemplates.removeElement(this, true);
	    return true;
	}
	else
	{
	    return false;
	}
    }

    public java.util.List findAllInstances(CommonBase child)
    {
	java.util.List result = super.findAllInstances(child);
	
	java.util.List serviceInstances = 
	    getApplication().findServiceInstances(_id);

	java.util.Iterator p = serviceInstances.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    result.addAll(service.findChildrenWithType(child.getClass()));
	}

	return result;
    }

    public Object saveDescriptor()
    {
	//
	// Shallow copy
	//
	TemplateDescriptor clone = (TemplateDescriptor)_templateDescriptor.clone();
	clone.descriptor = (ServiceDescriptor)_templateDescriptor.descriptor.clone();
	return clone;
    }
    
    public void restoreDescriptor(Object savedDescriptor)
    {
	TemplateDescriptor clone = (TemplateDescriptor)savedDescriptor;
	//
	// Keep the same object
	//
	_templateDescriptor.parameters = clone.parameters;

	ServiceDescriptor sd = (ServiceDescriptor)_templateDescriptor.descriptor;
	ServiceDescriptor csd = (ServiceDescriptor)clone.descriptor;

	sd.properties = csd.properties;
	sd.description = csd.description;
	sd.name = csd.name;
	sd.entry = csd.entry;
    }

    private TemplateDescriptor _templateDescriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;
    private final boolean _ephemeral;
    
    static private DefaultTreeCellRenderer _cellRenderer;  
    static private ServiceTemplateEditor _editor;
    static private JPopupMenu _popup;
}
