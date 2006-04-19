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
    //
    // VerifyInfo contains information that may be of use to a
    // CertificateVerifier implementation.
    //
    public struct VerifyInfo
    {
	//
	// A value of true indicates an incoming (server) connection.
	//
	public bool incoming;

	//
	// The peer's certificate. This value may be null if the peer
	// did not supply a certificate.
	//
	public System.Security.Cryptography.X509Certificates.X509Certificate cert;

	//
	// The chain of certificate authorities associated with the peer's
	// certificate.
	//
	public System.Security.Cryptography.X509Certificates.X509Chain chain;

	//
	// The SSL stream that is being authenticated.
	//
	public System.Net.Security.SslStream stream;

	//
	// The address of the server as specified by the proxy's
	// endpoint. For example, in the following proxy:
	//
	// identity:ssl -h www.server.com -p 10000
	//
	// the value of address is "www.server.com".
	//
	// The value is an empty string for incoming connections.
	//
	public string address;
    }

    //
    // An application can customize the certificate verification process
    // by implementing the CertificateVerifier interface.
    //
    public interface CertificateVerifier
    {
	//
	// Return true to allow a connection using the provided certificate
	// information, or false to reject the connection.
	//
	bool verify(VerifyInfo info);
    }

    abstract public class Plugin : Ice.LocalObjectImpl, Ice.Plugin
    {
	//
	// Manually initialize the plugin. The application must set the property
	// IceSSL.DelayInit=1 in order to use this method.
	//
	// It is legal to pass null as either argument, in which case the plugin
	// obtains its certificates as directed by configuration properties.
	//
	abstract public void initialize(X509Certificate2Collection certs);

	//
	// Establish the certificate verifier object. This should be
	// done before any connections are established.
	//
	abstract public void setCertificateVerifier(CertificateVerifier verifier);

	//
	// This method is for internal use.
	//
	abstract public void destroy();
    }
}
