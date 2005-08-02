// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.TemplateDescriptor;
import IceGrid.Model;

class ServiceTemplate extends Parent
{
    ServiceTemplate(String name, TemplateDescriptor descriptor, Model model)
    {
	super(name, model);
	rebuild(descriptor);
    }

    void rebuild(TemplateDescriptor descriptor)
    {
	_descriptor = descriptor;
	clearChildren();

	//
	// Fix-up parameters order
	//
	java.util.Collections.sort(_descriptor.parameters);
	
	_adapters = new Adapters(_descriptor.descriptor.adapters, true, 
				 null, _model);
	addChild(_adapters);

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, true,
			     null, _model);
	addChild(_dbEnvs);
    }

    public String toString()
    {
	return templateLabel(_id, _descriptor.parameters);
    }

    private TemplateDescriptor _descriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
