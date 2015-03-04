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
    using System.Text.RegularExpressions;

    public sealed class EndpointFactoryManager
    {
        internal EndpointFactoryManager(Instance instance)
        {
            instance_ = instance;
            _factories = new List<EndpointFactory>();
        }
        
        public void add(EndpointFactory factory)
        {
            lock(this)
            {
                for(int i = 0; i < _factories.Count; i++)
                {
                    EndpointFactory f = (EndpointFactory)_factories[i];
                    if(f.type() == factory.type())
                    {
                        Debug.Assert(false);
                    }
                }
                _factories.Add(factory);
            }
        }
        
        public EndpointFactory get(short type)
        {
            lock(this)
            {
                for(int i = 0; i < _factories.Count; i++)
                {
                    EndpointFactory f = (EndpointFactory)_factories[i];
                    if(f.type() == type)
                    {
                        return f;
                    }
                }
                return null;
            }
        }
        
        public EndpointI create(string str, bool oaEndpoint)
        {
            lock(this)
            {
                string s = str.Trim();
                if(s.Length == 0)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "value has no non-whitespace characters";
                    throw e;
                }
                
                Regex p = new Regex("([ \t\n\r]+)|$");
                Match m = p.Match(s);
                Debug.Assert(m.Success);
                
                string protocol = s.Substring(0, m.Index);
                
                if(protocol.Equals("default"))
                {
                    protocol = instance_.defaultsAndOverrides().defaultProtocol;
                }
                
                for(int i = 0; i < _factories.Count; i++)
                {
                    EndpointFactory f = (EndpointFactory)_factories[i];
                    if(f.protocol().Equals(protocol))
                    {
                        return f.create(s.Substring(m.Index + m.Length), oaEndpoint);

                        // Code below left in place for debugging.

                        /*
                        EndpointI e = f.create(s.Substring(m.Index + m.Length), oaEndpoint);
                        BasicStream bs = new BasicStream(instance_, true);
                        e.streamWrite(bs);
                        Buffer buf = bs.getBuffer();
                        buf.b.position(0);
                        short type = bs.readShort();
                        EndpointI ue = new IceInternal.OpaqueEndpointI(type, bs);
                        System.Console.Error.WriteLine("Normal: " + e);
                        System.Console.Error.WriteLine("Opaque: " + ue);
                        return e;
                        */
                    }
                }

                //
                // If the stringified endpoint is opaque, create an unknown endpoint,
                // then see whether the type matches one of the known endpoints.
                //
                if(protocol.Equals("opaque"))
                {
                    EndpointI ue = new OpaqueEndpointI(s.Substring(m.Index + m.Length));
                    for(int i = 0; i < _factories.Count; i++)
                    {
                        EndpointFactory f = (EndpointFactory)_factories[i];
                        if(f.type() == ue.type())
                        {
                            //
                            // Make a temporary stream, write the opaque endpoint data into the stream,
                            // and ask the factory to read the endpoint data from that stream to create
                            // the actual endpoint.
                            //
                            BasicStream bs = new BasicStream(instance_, Ice.Util.currentProtocolEncoding, true);
                            ue.streamWrite(bs);
                            Buffer buf = bs.getBuffer();
                            buf.b.position(0);
                            bs.readShort(); // type
                            return f.read(bs);
                        }
                    }
                    return ue; // Endpoint is opaque, but we don't have a factory for its type.
                }
                return null;
            }
        }
        
        public EndpointI read(BasicStream s)
        {
            lock(this)
            {
                short type = s.readShort();
                
                for(int i = 0; i < _factories.Count; i++)
                {
                    EndpointFactory f = (EndpointFactory)_factories[i];
                    if(f.type() == type)
                    {
                        return f.read(s);
                    }
                }
                
                return new OpaqueEndpointI(type, s);
            }
        }
        
        internal void destroy()
        {
            for(int i = 0; i < _factories.Count; i++)
            {
                EndpointFactory f = (EndpointFactory)_factories[i];
                f.destroy();
            }
            _factories.Clear();
        }
        
        private readonly Instance instance_;
        private readonly List<EndpointFactory> _factories;
    }

}
