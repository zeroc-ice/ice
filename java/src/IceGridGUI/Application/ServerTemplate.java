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

import IceGrid.*;
import IceGridGUI.*;


class ServerTemplate extends Communicator
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
	if(((TreeNode)_parent).getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}

	actions[DELETE] = true;

	actions[NEW_ADAPTER] = !_services.initialized();
	actions[NEW_SERVICE] = _services.initialized();
	actions[NEW_SERVICE_FROM_TEMPLATE] = _services.initialized();
	actions[NEW_DBENV] = _dbEnvs.initialized();

	return actions;
    }
    public void copy()
    {
	getCoordinator().setClipboard(copyDescriptor(_templateDescriptor));
	getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }
    public void paste()
    {
	((TreeNode)_parent).paste();
    }
   
    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = getCoordinator().getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_ADAPTER));
	    _popup.add(actions.get(NEW_DBENV));
	    _popup.add(actions.get(NEW_SERVICE));
	    _popup.add(actions.get(NEW_SERVICE_FROM_TEMPLATE));
	}
	actions.setTarget(this);
	return _popup;
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = (ServerTemplateEditor)getRoot().
		getEditor(ServerTemplateEditor.class, this);
	}
	_editor.show(this);
	return _editor;
    }

    protected Editor createEditor()
    {
	return new ServerTemplateEditor(getCoordinator().getMainFrame());
    }
    
    public void destroy()
    {
	ServerTemplates serverTemplates = (ServerTemplates)_parent;

	if(_ephemeral)
	{
	    serverTemplates.removeChild(this);
	}
	else
	{
	    serverTemplates.removeDescriptor(_id);
	    getRoot().removeServerInstances(_id);
	    serverTemplates.removeChild(this);
	    serverTemplates.getEditable().removeElement(_id);
	    getRoot().updated();
	}
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public Object getDescriptor()
    {
	return _templateDescriptor;
    }

    CommunicatorDescriptor getCommunicatorDescriptor()
    {
	return _templateDescriptor.descriptor;
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
    ServerTemplate(boolean brandNew, ServerTemplates parent, String name, TemplateDescriptor descriptor)
	throws UpdateFailedException
    {
	super(parent, name);
	_editable = new Editable(brandNew);
	_ephemeral = false;
	rebuild(descriptor);
    }

    ServerTemplate(ServerTemplates parent, String name, TemplateDescriptor descriptor)
    {
	super(parent, name);
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
    
    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    java.util.List attributes = new java.util.LinkedList();
	    attributes.add(createAttribute("id", _id));
	    writer.writeStartTag("server-template", attributes);
	    writeParameters(writer, _templateDescriptor.parameters,
			    _templateDescriptor.parameterDefaults);
	    
	    if(_templateDescriptor.descriptor instanceof IceBoxDescriptor)
	    {
		IceBoxDescriptor descriptor = (IceBoxDescriptor)_templateDescriptor.descriptor;

		writer.writeStartTag("icebox", 
				     Server.createAttributes(descriptor));

		if(descriptor.description.length() > 0)
		{
		    writer.writeElement("description", descriptor.description);
		}
		Server.writeOptions(writer, descriptor.options);
		Server.writeEnvs(writer, descriptor.envs);
		writeProperties(writer, descriptor.properties);
		writeDistribution(writer, descriptor.distrib);

		_adapters.write(writer);
		_services.write(writer);
		writer.writeEndTag("icebox");
	    }
	    else
	    {
		ServerDescriptor descriptor = (ServerDescriptor)_templateDescriptor.descriptor;

		writer.writeStartTag("server", 
				     Server.createAttributes(descriptor));

		if(descriptor.description.length() > 0)
		{
		    writer.writeElement("description", descriptor.description);
		}
		Server.writeOptions(writer, descriptor.options);
		Server.writeEnvs(writer, descriptor.envs);
		writeProperties(writer, descriptor.properties);
		writeDistribution(writer, descriptor.distrib);

		_adapters.write(writer);
		_dbEnvs.write(writer);
		writer.writeEndTag("server");
	    }  
	    writer.writeEndTag("server-template");
	}
    }
    

    boolean isIceBox()
    {
	return _templateDescriptor.descriptor instanceof IceBoxDescriptor;
    }

    void rebuild(TemplateDescriptor descriptor) throws UpdateFailedException
    {
	_templateDescriptor = descriptor;

	_adapters.clear();
	_dbEnvs.clear();
	_services.clear();

	if(!_ephemeral)
	{
	    _adapters.init(_templateDescriptor.descriptor.adapters);
	    
	    if(_templateDescriptor.descriptor instanceof IceBoxDescriptor)
	    {
		IceBoxDescriptor iceBoxDescriptor = 
		    (IceBoxDescriptor)_templateDescriptor.descriptor;
		
		_services.init(iceBoxDescriptor.services);
		
		assert _templateDescriptor.descriptor.dbEnvs.size() == 0;
	    }
	    else
	    {
		_dbEnvs.init(_templateDescriptor.descriptor.dbEnvs);
	    }
	}
    }

    void rebuild() throws UpdateFailedException
    {
	rebuild(_templateDescriptor);
    }

    void commit()
    {
	_editable.commit();
    }
    
    Editable getEditable()
    {
	return _editable;
    }

    Editable getEnclosingEditable()
    {
	return _editable;
    }

    java.util.List findInstances(boolean includeTemplate)
    {
	java.util.List result = getRoot().findServerInstances(_id);
	if(includeTemplate)
	{
	    result.add(0, this);
	}
	return result;
    }

    private TemplateDescriptor _templateDescriptor;
    private final boolean _ephemeral;
    private Editable _editable;
    private ServerTemplateEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _plainIcon;
    static private Icon _iceboxIcon;
  
    static private JPopupMenu _popup;
}
