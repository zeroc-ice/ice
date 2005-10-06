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

class UpdateFailedException extends Exception
{
    UpdateFailedException(Parent parent, String duplicateName)
    {
	_duplicateName = duplicateName;
	_parentList = new java.util.LinkedList();
	_parentList.addFirst(parent);
    }
    
    UpdateFailedException(String message)
    {
	_message = message;
    }

    void addParent(Parent parent)
    {
	if(_message == null)
	{
	    Parent firstParent = (Parent)_parentList.get(0);
	    if(firstParent != parent && firstParent.getParent() == null)
	    {
		_parentList.addFirst(parent);
	    }
	}
    }    

    public String toString()
    {
	if(_message == null)
	{
	    _message = "";
	    if(_parentList != null)
	    {
		TreePath path = null;

		java.util.Iterator p = _parentList.iterator();
		while(p.hasNext())
		{
		    Parent parent = (Parent)p.next();
		    if(path == null)
		    {
			path = parent.getPath();
			if(path == null)
			{
			    path = new TreePath(parent);
			}
		    }
		    else
		    {
			path = path.pathByAddingChild(parent);
		    }
		}
		
		for(int i = 1; i < path.getPathCount(); ++i)
		{
		    if(!_message.equals(""))
		    {
			_message += "/";
		    }
		    _message += ((CommonBase)path.getPathComponent(i)).getId();
		}
	    }
	    else
	    {
		_message = "???";
	    }
	    _message += " cannot have two children named " + _duplicateName;
	}
	return _message; 
    }
    
    private String _message;
    private String _duplicateName;
    private java.util.LinkedList _parentList;
}
