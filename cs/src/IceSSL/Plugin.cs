// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    abstract public class Plugin : Ice.LocalObjectImpl, Ice.Plugin
    {
	//
	// Manually initialize the plugin. The application must set the property
	// IceSSL.DelayInit=1 in order to use this method.
	//
	// It is legal to pass null as either argument, in which case the plugin
	// obtains its certificates as directed by configuration properties.
	//
	abstract public void initialize(X509Certificate2Collection clientCerts,
					X509Certificate2 serverCert);

	abstract public void destroy();
    }
}
