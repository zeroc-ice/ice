// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class Instance
    {
        public bool destroyed()
        {
            return _state == StateDestroyed;
        }

        public Ice.InitializationData initializationData()
        {
            //
            // No check for destruction. It must be possible to access the
            // initialization data after destruction.
            //
            // No mutex lock, immutable.
            //
            return _initData;
        }
        
        public TraceLevels traceLevels()
        {
            // No mutex lock, immutable.
            return _traceLevels;
        }
        
        public DefaultsAndOverrides defaultsAndOverrides()
        {
            // No mutex lock, immutable.
            return _defaultsAndOverrides;
        }

        public System.Uri bridgeUri()
        {
            return _bridgeUri;
        }

        public ReferenceFactory referenceFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _referenceFactory;
            }
        }
        
        public ProxyFactory proxyFactory()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _proxyFactory;
            }
        }
        
        public ObjectFactoryManager servantFactoryManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _servantFactoryManager;
            }
        }
        
        public EndpointFactoryManager endpointFactoryManager()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return _endpointFactoryManager;
            }
        }
        
        public int messageSizeMax()
        {
            // No mutex lock, immutable.
            return _messageSizeMax;
        }
        
        public void setDefaultContext(Dictionary<string, string> ctx)
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
        
                if(ctx == null || ctx.Count == 0)
                {
                    _defaultContext = _emptyContext;
                }
                else
                {
                    _defaultContext = new Dictionary<string, string>(ctx);
                }
            }
        }

        public Dictionary<string, string> getDefaultContext()
        {
            lock(this)
            {
                if(_state == StateDestroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                
                return new Dictionary<string, string>(_defaultContext);
            }
        }

        public Ice.ImplicitContextI getImplicitContext()
        {
            return _implicitContext;
        }

        public Ice.Identity stringToIdentity(string s)
        {
            return Ice.Util.stringToIdentity(s);
        }

        public string identityToString(Ice.Identity ident)
        {
            return Ice.Util.identityToString(ident);
        }

        //
        // Only for use by Ice.CommunicatorI
        //
        public Instance(Ice.Communicator communicator, Ice.InitializationData initData)
        {
            _state = StateActive;
            _initData = initData;
                
            try
            {
                if(_initData.properties == null)
                {
                    _initData.properties = Ice.Util.createProperties();
                }

                lock(_staticLock)
                {
                    if(!_oneOffDone)
                    {
                        string stdOut = _initData.properties.getProperty("Ice.StdOut");
                        string stdErr = _initData.properties.getProperty("Ice.StdErr");
                        
                        System.IO.StreamWriter outStream = null;
                        
                        if(stdOut.Length > 0)
                        {
                            try
                            {                       
                                outStream = System.IO.File.AppendText(stdOut);
                            }
                            catch(System.IO.IOException ex)
                            {
                                Ice.FileException fe = new Ice.FileException(ex);
                                fe.path = stdOut;
                                throw fe;       
                            }
                            outStream.AutoFlush = true;
                            System.Console.Out.Close();
                            System.Console.SetOut(outStream);
                        }
                        if(stdErr.Length > 0)
                        {
                            if(stdErr.Equals(stdOut))
                            {
                                System.Console.SetError(outStream); 
                            }
                            else
                            {
                                System.IO.StreamWriter errStream = null;
                                try
                                {
                                    errStream = System.IO.File.AppendText(stdErr);
                                }
                                catch(System.IO.IOException ex)
                                {
                                    Ice.FileException fe = new Ice.FileException(ex);
                                    fe.path = stdErr;
                                    throw fe;   
                                }
                                errStream.AutoFlush = true;
                                System.Console.Error.Close();
                                System.Console.SetError(errStream);
                            }
                        }

                        _oneOffDone = true;
                    }
                }
                
                if(_initData.logger == null)
                {
                    _initData.logger = Ice.Util.getProcessLogger();
                }
                
                _traceLevels = new TraceLevels(_initData.properties);
                
                _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties);
                
                {
                    const int defaultMessageSizeMax = 1024;
                    int num = 
                        _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
                    if(num < 1)
                    {
                        _messageSizeMax = defaultMessageSizeMax * 1024; // Ignore non-sensical values.
                    }
                    else if(num > 0x7fffffff / 1024)
                    {
                        _messageSizeMax = 0x7fffffff;
                    }
                    else
                    {
                        _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                    }
                }

                _bridgeUri = new System.Uri(_initData.properties.getPropertyWithDefault(
                    "Ice.BridgeUri", "http://127.0.0.1:8080/IceBridge.ashx"));
                
                _implicitContext = Ice.ImplicitContextI.create(_initData.properties.getProperty("Ice.ImplicitContext"));
                _referenceFactory = new ReferenceFactory(this, communicator);
                
                _proxyFactory = new ProxyFactory(this);
                
                _endpointFactoryManager = new EndpointFactoryManager(this);
                EndpointFactory tcpEndpointFactory = new TcpEndpointFactory(this);
                _endpointFactoryManager.add(tcpEndpointFactory);
                EndpointFactory udpEndpointFactory = new UdpEndpointFactory(this);
                _endpointFactoryManager.add(udpEndpointFactory);
                
                _defaultContext = _emptyContext;
                
                _servantFactoryManager = new ObjectFactoryManager();
            }
            catch(Ice.LocalException)
            {
                destroy();
                throw;
            }
        }
        
        public void finishSetup(ref string[] args)
        {
            //
            // Load plug-ins.
            //
            if(_initData.logger == null)
            {
                _initData.logger = new Ice.LoggerI(_initData.properties.getProperty("Ice.ProgramName"));
            }
        }
        
        //
        // Only for use by Ice.CommunicatorI
        //
        public bool destroy()
        {
            lock(this)
            {
                //
                // If the _state is not StateActive then the instance is
                // either being destroyed, or has already been destroyed.
                //
                if(_state != StateActive)
                {
                    return false;
                }
            
                //
                // We cannot set state to StateDestroyed otherwise instance
                // methods called during the destroy process (such as
                // outgoingConnectionFactory() from
                // ObjectAdapterI::deactivate() will cause an exception.
                //
                // XXX:
                //
                _state = StateDestroyInProgress;
            }
            
            lock(this)
            {
                if(_servantFactoryManager != null)
                {
                    _servantFactoryManager.destroy();
                    _servantFactoryManager = null;
                }
                
                if(_referenceFactory != null)
                {
                    _referenceFactory.destroy();
                    _referenceFactory = null;
                }
                
                // No destroy function defined.
                // _proxyFactory.destroy();
                _proxyFactory = null;
                
                if(_endpointFactoryManager != null)
                {
                    _endpointFactoryManager.destroy();
                    _endpointFactoryManager = null;
                }
                
                _state = StateDestroyed;
            }
            
            if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
            {
                ArrayList unusedProperties = ((Ice.PropertiesI)_initData.properties).getUnusedProperties();
                if(unusedProperties.Count != 0)
                {
                    string message = "The following properties were set but never read:";
                    foreach(string s in unusedProperties)
                    {
                        message += "\n    " + s;
                    }
                    _initData.logger.warning(message);
                }
            }

            return true;
        }
        
        private const int StateActive = 0;
        private const int StateDestroyInProgress = 1;
        private const int StateDestroyed = 2;
        private System.Uri _bridgeUri;
        private int _state;
        private Ice.InitializationData _initData; // Immutable, not reset by destroy().
        private TraceLevels _traceLevels; // Immutable, not reset by destroy().
        private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
        private int _messageSizeMax; // Immutable, not reset by destroy().
        private Ice.ImplicitContextI _implicitContext; // Immutable
        private ObjectFactoryManager _servantFactoryManager;
        private ReferenceFactory _referenceFactory;
        private ProxyFactory _proxyFactory;
        private EndpointFactoryManager _endpointFactoryManager;
        private Dictionary<string, string> _defaultContext;

        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

        private static bool _oneOffDone = false;
        private static System.Object _staticLock = new System.Object();

    }

}
