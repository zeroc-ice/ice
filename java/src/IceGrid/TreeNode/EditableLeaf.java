// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;
import IceGrid.Model;

class EditableLeaf extends Leaf implements Editable
{
    public boolean isNew()
    {
	return _isNew;
    }
    
    public boolean isModified()
    {
	return _modified;
    }

    public void markModified()
    {
	_modified = true;
    }

    public Editable getEditable()
    {
	return this;
    }
    
    public void commit()
    {
	_isNew = false;
	_modified = false;
    }

    protected EditableLeaf(boolean brandNew, String id, Model model)
    {
	super(id, model);
	_isNew = brandNew;
    }

    private boolean _isNew;
    private boolean _modified;
}
