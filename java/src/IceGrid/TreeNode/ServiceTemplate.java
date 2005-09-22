// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.TemplateDescriptor;
import IceGrid.Model;

class ServiceTemplate extends EditableParent
{
    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties for " + _id);
       
	if(_editor == null)
	{
	    _editor = new ServiceTemplateEditor(_model.getMainFrame());
	}
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }


    ServiceTemplate(boolean brandNew, String name, 
		    TemplateDescriptor descriptor, Model model)
	throws DuplicateIdException
    {
	super(brandNew, name, model);
	rebuild(descriptor);
    }
    
    ServiceTemplate(ServiceTemplate o)
    {
	super(o, true);
	_templateDescriptor = o._templateDescriptor;
	_adapters = o._adapters;
	_dbEnvs = o._dbEnvs;
	_propertiesHolder = o._propertiesHolder;
    }


    void rebuild(TemplateDescriptor descriptor)
	throws DuplicateIdException
    {
	_templateDescriptor = descriptor;
	_propertiesHolder = new PropertiesHolder(_templateDescriptor.descriptor);
	clearChildren();

	//
	// Fix-up parameters order
	//
	java.util.Collections.sort(_templateDescriptor.parameters);
	
	_adapters = new Adapters(_templateDescriptor.descriptor.adapters, true, 
				 false, null, null, _model);
	addChild(_adapters);
	_adapters.setParent(this);

	_dbEnvs = new DbEnvs(_templateDescriptor.descriptor.dbEnvs, true,
			     null, _model);
	addChild(_dbEnvs);
	_dbEnvs.setParent(this);
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public String toString()
    {
	return templateLabel(_id, _templateDescriptor.parameters);
    }

    public Object getDescriptor()
    {
	return _templateDescriptor;
    }

    private TemplateDescriptor _templateDescriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;

    static private ServiceTemplateEditor _editor;
}
