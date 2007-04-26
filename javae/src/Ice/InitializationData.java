// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class InitializationData
{
    public InitializationData()
    {
    }
    
    public java.lang.Object
    ice_clone()
    {
	InitializationData clone = new InitializationData();
	clone.properties = properties;
	clone.logger = logger;
	return clone;
    }
    

    public Properties properties;
    public Logger logger;
}
