// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Ice.PluginFactory;
import Ice.LoggerPlugin;

public class LoggerPluginFactoryI implements Ice.PluginFactory
{
     public Ice.Plugin
     create(Ice.Communicator communicator, String name, String[] args)
     {
         return new Ice.LoggerPlugin(communicator, new LoggerI());
     }
}
