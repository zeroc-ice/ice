// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.tree.TreePath;

import IceGrid.Actions;
import IceGrid.ServiceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Model;

class ServiceTemplateActions extends Actions
{
    ServiceTemplateActions(Model model)
    {
	super(model);
    }
    
    void reset(ServiceTemplate serviceTemplate)
    {
	_serviceTemplate = serviceTemplate;
	_copy.setEnabled(true);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof TemplateDescriptor)
	{
	    TemplateDescriptor d = (TemplateDescriptor)clipboard;
	    _paste.setEnabled(d.descriptor instanceof ServiceDescriptor);
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(true);
    }

    protected void copy()
    {
	_model.setClipboard(_serviceTemplate.copy());
	_paste.setEnabled(true);
    }

    protected void paste()
    {
	((ServiceTemplates)_serviceTemplate.getParent()).paste();
    }

    protected void delete()
    {
	TreePath parentPath = _serviceTemplate.getParent().getPath();
	_serviceTemplate.destroy();
	_model.setSelectionPath(parentPath);
    }

    ServiceTemplate _serviceTemplate; 
}
