// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;


class NodeObserverI extends _NodeObserverDisp
{
    NodeObserverI(Model model)
    {
	_model = model;
    }
    
    public void init(NodeDynamicInfo[] nodes, Ice.Current current)
    {
    }

    public void initNode(NodeDynamicInfo node, Ice.Current current)
    {
    }

    public void updateServer(String node, ServerDynamicInfo updatedInfo, Ice.Current current)
    {
    }

    public void updateAdapter(String node, AdapterDynamicInfo updatedInfo, Ice.Current current)
    {
    }

    private Model _model;
};
