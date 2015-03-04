// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class ConnectionReaper
    {
        public void
        add(Ice.ConnectionI connection, Ice.Instrumentation.Observer observer)
        {
            lock(this)
            {
                _connections.Add(connection);
                if(observer != null)
                {
                    observer.detach();
                }
            }
        }

        public ICollection<Ice.ConnectionI>
        swapConnections()
        {
            lock(this)
            {
                if(_connections.Count == 0)
                {
                    return null;
                }

                ICollection<Ice.ConnectionI> tmp = _connections;
                _connections = new List<Ice.ConnectionI>();
                return tmp;
            }
        }

        private ICollection<Ice.ConnectionI> _connections = new List<Ice.ConnectionI>();
    }
}
