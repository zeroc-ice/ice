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

class ServiceTemplate extends Parent
{
    ServiceTemplate(String name, TemplateDescriptor descriptor)
    {
	_name = name;
	_descriptor = descriptor;
	
	_adapters = new Adapters(_descriptor.descriptor.adapters, true);
	addChild(_adapters);

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, true);
	addChild(_dbEnvs);
    }

    public String toString()
    {
	return templateLabel(_name, _descriptor.parameters);
    }

    private String _name;
    private TemplateDescriptor _descriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
