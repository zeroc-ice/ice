// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Security.Cryptography.X509Certificates;

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
	// From Ice.Plugin.
	//
	abstract public void initialize();

	//
	// Specify the certificates to use for SSL connections. This
	// must be done before the plugin is initialized, therefore
	// the application must define the property Ice.InitPlugins=0,
	// set the certificates, and finally invoke initializePlugins
	// on the PluginManager.
	//
	// When the application supplies its own certificates, the
	// plugin skips its normal property-based configuration.
	//
	abstract public void setCertificates(X509Certificate2Collection certs);

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
