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
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

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
    
    //
    // Application is needed to lookup service templates
    //
    ServerTemplate(boolean brandNew, String name, TemplateDescriptor descriptor, 
		   Application application)
	throws DuplicateIdException
    {
	super(brandNew, name, application.getModel());
	rebuild(descriptor, application);
    }

    ServerTemplate(ServerTemplate o)
    {
	super(o, true);
	_templateDescriptor = o._templateDescriptor;
	_iceBoxDescriptor = o._iceBoxDescriptor;
	_services = o._services;
	_dbEnvs = o._dbEnvs;
	_adapters = o._adapters;
	
	_propertiesHolder = o._propertiesHolder;
    }

    void rebuild(TemplateDescriptor descriptor, Application application)
	throws DuplicateIdException
    {
	_templateDescriptor = descriptor;
	_propertiesHolder = new PropertiesHolder(_templateDescriptor.descriptor);
	clearChildren();

	//
	// Fix-up parameters order
	//
	java.util.Collections.sort(_templateDescriptor.parameters);
	
	if(_templateDescriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_templateDescriptor.descriptor;
	    
	    _services = new Services(_iceBoxDescriptor.services, this, null, 
				     application);
	    addChild(_services);
	    _services.setParent(this);

	    assert _templateDescriptor.descriptor.dbEnvs.size() == 0;
	    _dbEnvs = null;
	}
	else
	{
	    _services = null;
	    _iceBoxDescriptor = null;

	    _dbEnvs = new DbEnvs(_templateDescriptor.descriptor.dbEnvs, true,
			     null, _model);
	    addChild(_dbEnvs);
	    _dbEnvs.setParent(this);
	}
	
	_adapters = new Adapters(_templateDescriptor.descriptor.adapters, true, 
				 null, null, _model);
	addChild(_adapters);
	_adapters.setParent(this);
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public String toString()
    {
	return templateLabel(_id, _templateDescriptor.parameters);
    }

    public TemplateDescriptor getDescriptor()
    {
	return _templateDescriptor;
    }

    private TemplateDescriptor _templateDescriptor;
    private IceBoxDescriptor _iceBoxDescriptor;

    private Services _services;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;
}
