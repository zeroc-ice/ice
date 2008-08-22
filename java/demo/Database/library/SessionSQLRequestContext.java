// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class SessionSQLRequestContext extends SQLRequestContext
{
    SessionSQLRequestContext(SessionI session, Ice.Logger logger, ConnectionPool pool)
    {
        super(logger, pool);
        _session = session;
    }
    
    SessionI getSession()
    {
        return _session;
    }
    
    private SessionI _session;
}
