// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;
import IceGrid.Model;

class EditableLeaf extends Leaf implements Editable
{
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

    protected EditableLeaf(boolean brandNew, String id, Model model)
    {
	super(id, model);
	_modified = brandNew;
    }

    protected EditableLeaf(EditableLeaf o)
    {
	super(o);
	_modified = o._modified;
    }


    private boolean _modified;
}
