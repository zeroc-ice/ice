<%@ Language="c#" %>

<script runat="server">
// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
void Application_Start(Object sender, EventArgs e)
{
    //Ice.InitializationData initData = new Ice.InitializationData();
    //initData.properties = Ice.Util.createProperties();
    //initData.properties.load("ice.config");
    //Application["com"] = Ice.Util.initialize(initData);
    Application["com"] = Ice.Util.initialize();
}

void Application_End(Object sender, EventArgs e)
{
    Ice.Communicator com = (Ice.Communicator)Application["com"];
    try
    {
        com.destroy();
    }
    catch(Exception)
    {
    }
}
</script>
