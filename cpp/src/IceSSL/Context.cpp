// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Context.h>
#include <Instance.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>

#include <openssl/x509v3.h>
#include <openssl/err.h>

#include <IceUtil/DisableWarnings.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

static int
opensslPasswordCallback(char* buf, int size, int flag, void* userData)
{
    IceSSL::Context* c = reinterpret_cast<IceSSL::Context*>(userData);
    string passwd = c->password(flag == 1);
    int sz = static_cast<int>(passwd.size());
    if(sz > size)
    {
	sz = size - 1;
    }
    strncpy(buf, passwd.c_str(), sz);
    buf[sz] = '\0';
    return sz;
}

#ifndef OPENSSL_NO_DH
static DH*
opensslDHCallback(SSL* ssl, int /*isExport*/, int keyLength)
{
    IceSSL::Context* c = reinterpret_cast<IceSSL::Context*>(SSL_CTX_get_ex_data(ssl->ctx, 0));
    return c->dhParams(keyLength);
}
#endif

static int
opensslVerifyCallback(int ok, X509_STORE_CTX* ctx)
{
    SSL* ssl = reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    IceSSL::Context* c = reinterpret_cast<IceSSL::Context*>(SSL_CTX_get_ex_data(ssl->ctx, 0));
    return c->verifyCallback(ok, ssl, ctx);
}

static bool
passwordError()
{
    int reason = ERR_GET_REASON(ERR_peek_error());
    return (reason == PEM_R_BAD_BASE64_DECODE ||
	    reason == PEM_R_BAD_DECRYPT ||
	    reason == PEM_R_BAD_PASSWORD_READ ||
	    reason == PEM_R_PROBLEMS_GETTING_PASSWORD);
}

//
// Context.
//
IceSSL::Context::Context(const InstancePtr& instance, SSL_CTX* ctx) :
    _instance(instance),
    _logger(instance->communicator()->getLogger()),
    _ctx(ctx)
{
    if(_ctx)
    {
	return;
    }

    _ctx = SSL_CTX_new(SSLv23_method());
    if(!_ctx)
    {
	PluginInitializationException ex(__FILE__, __LINE__);
	ex.reason = "IceSSL: unable to create SSL context:\n" + _instance->sslErrors();
	throw ex;
    }

    try
    {
	//
	// Store a pointer to ourself for use in OpenSSL callbacks.
	//
	SSL_CTX_set_ex_data(_ctx, 0, this);

	//
	// This is necessary for successful interop with Java. Without it, a Java
	// client would fail to reestablish a connection: the server gets the
	// error "session id context uninitialized" and the client receives
	// "SSLHandshakeException: Remote host closed connection during handshake".
	//
	SSL_CTX_set_session_cache_mode(_ctx, SSL_SESS_CACHE_OFF);

	PropertiesPtr properties = _instance->communicator()->getProperties();
	const string propPrefix = "IceSSL.";

	//
	// Check for a default directory. We look in this directory for
	// files mentioned in the configuration.
	//
	{
	    _defaultDir = properties->getProperty(propPrefix + "DefaultDir");
	}

	//
	// Select protocols.
	//
	{
	    string protocols = properties->getProperty(propPrefix + "Protocols");
	    if(!protocols.empty())
	    {
		parseProtocols(protocols);
	    }
	}

	//
	// CheckCertName determines whether we compare the name in a peer's
	// certificate against its hostname.
	//
	{
	    _checkCertName = properties->getPropertyAsIntWithDefault(propPrefix + "CheckCertName", 0) > 0;
	}

	//
	// Determine whether a certificate is required from the peer.
	//
	{
	    int verifyPeer = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyPeer", 2);
	    int sslVerifyMode;
	    switch(verifyPeer)
	    {
	    case 0:
		sslVerifyMode = SSL_VERIFY_NONE;
		break;
	    case 1:
		sslVerifyMode = SSL_VERIFY_PEER;
		break;
	    case 2:
		sslVerifyMode = SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
		break;
	    default:
	    {
		PluginInitializationException ex(__FILE__, __LINE__);
		ex.reason = "IceSSL: invalid value for " + propPrefix + "VerifyPeer";
		throw ex;
	    }
	    }
	    SSL_CTX_set_verify(_ctx, sslVerifyMode, opensslVerifyCallback);
	}

	//
	// If the configuration defines a password, or the application has supplied
	// a password prompt object, then register a password callback. Otherwise,
	// let OpenSSL use its default behavior.
	//
	{
	    // TODO: Support quoted value?
	    string password = properties->getProperty(propPrefix + "Password");
	    if(!password.empty() || _instance->passwordPrompt())
	    {
		SSL_CTX_set_default_passwd_cb(_ctx, opensslPasswordCallback);
		SSL_CTX_set_default_passwd_cb_userdata(_ctx, this);
		_password = password;
	    }
	}

	int passwordRetryMax = properties->getPropertyAsIntWithDefault(propPrefix + "PasswordRetryMax", 3);

	//
	// Establish the location of CA certificates.
	//
	{
	    string caFile = properties->getProperty(propPrefix + "CertAuthFile");
	    string caDir = properties->getPropertyWithDefault(propPrefix + "CertAuthDir", _defaultDir);
	    const char* file = 0;
	    const char* dir = 0;
	    if(!caFile.empty())
	    {
		if(!checkPath(caFile, _defaultDir, false))
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: CA certificate file not found:\n" + caFile;
		    throw ex;
		}
		file = caFile.c_str();
	    }
	    if(!caDir.empty())
	    {
		if(!checkPath(caDir, _defaultDir, true))
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: CA certificate directory not found:\n" + caDir;
		    throw ex;
		}
		dir = caDir.c_str();
	    }
	    if(file || dir)
	    {
		//
		// The certificate may be stored in an encrypted file, so handle
		// password retries.
		//
		int count = 0;
		int err;
		while(count < passwordRetryMax)
		{
		    ERR_clear_error();
		    err = SSL_CTX_load_verify_locations(_ctx, file, dir);
		    if(err || !passwordError())
		    {
			break;
		    }
		    ++count;
		}
		if(err == 0)
		{
		    string msg = "IceSSL: unable to establish CA certificates";
		    if(passwordError())
		    {
			msg += ":\ninvalid password";
		    }
		    else
		    {
			string err = _instance->sslErrors();
			if(!err.empty())
			{
			    msg += ":\n" + err;
			}
		    }
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = msg;
		    throw ex;
		}
	    }
	}

	//
	// Establish the certificate chains and private keys. One RSA certificate and
	// one DSA certificate are allowed.
	//
	{
#ifdef _WIN32
	    const string sep = ";";
#else
	    const string sep = ":";
#endif
	    string certFile = properties->getProperty(propPrefix + "CertFile");
	    string keyFile = properties->getProperty(propPrefix + "KeyFile");
	    vector<string>::size_type numCerts = 0;
	    if(!certFile.empty())
	    {
		vector<string> files;
		if(!splitString(certFile, sep, false, files) || files.size() > 2)
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: invalid value for " + propPrefix + "CertFile:\n" + certFile;
		    throw ex;
		}
		numCerts = files.size();
		for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
		{
		    string file = *p;
		    if(!checkPath(file, _defaultDir, false))
		    {
			PluginInitializationException ex(__FILE__, __LINE__);
			ex.reason = "IceSSL: certificate file not found:\n" + file;
			throw ex;
		    }
		    //
		    // The certificate may be stored in an encrypted file, so handle
		    // password retries.
		    //
		    int count = 0;
		    int err;
		    while(count < passwordRetryMax)
		    {
			ERR_clear_error();
			err = SSL_CTX_use_certificate_chain_file(_ctx, file.c_str());
			if(err || !passwordError())
			{
			    break;
			}
			++count;
		    }
		    if(err == 0)
		    {
			string msg = "IceSSL: unable to load certificate chain from file " + file;
			if(passwordError())
			{
			    msg += ":\ninvalid password";
			}
			else
			{
			    string err = _instance->sslErrors();
			    if(!err.empty())
			    {
				msg += ":\n" + err;
			    }
			}
			PluginInitializationException ex(__FILE__, __LINE__);
			ex.reason = msg;
			throw ex;
		    }
		}
	    }
	    if(keyFile.empty())
	    {
		keyFile = certFile; // Assume the certificate file also contains the private key.
	    }
	    if(!keyFile.empty())
	    {
		vector<string> files;
		if(!splitString(keyFile, sep, false, files) || files.size() > 2)
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: invalid value for " + propPrefix + "KeyFile:\n" + keyFile;
		    throw ex;
		}
		if(files.size() != numCerts)
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: " + propPrefix + "KeyFile does not agree with " + propPrefix + "CertFile";
		    throw ex;
		}
		for(vector<string>::iterator p = files.begin(); p != files.end(); ++p)
		{
		    string file = *p;
		    if(!checkPath(file, _defaultDir, false))
		    {
			PluginInitializationException ex(__FILE__, __LINE__);
			ex.reason = "IceSSL: key file not found:\n" + file;
			throw ex;
		    }
		    //
		    // The private key may be stored in an encrypted file, so handle
		    // password retries.
		    //
		    int count = 0;
		    int err;
		    while(count < passwordRetryMax)
		    {
			ERR_clear_error();
			err = SSL_CTX_use_PrivateKey_file(_ctx, file.c_str(), SSL_FILETYPE_PEM);
			if(err || !passwordError())
			{
			    break;
			}
			++count;
		    }
		    if(err == 0)
		    {
			string msg = "IceSSL: unable to load private key from file " + file;
			if(passwordError())
			{
			    msg += ":\ninvalid password";
			}
			else
			{
			    string err = _instance->sslErrors();
			    if(!err.empty())
			    {
				msg += ":\n" + err;
			    }
			}
			PluginInitializationException ex(__FILE__, __LINE__);
			ex.reason = msg;
			throw ex;
		    }
		}
		if(!SSL_CTX_check_private_key(_ctx))
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: unable to validate private key(s):\n" + _instance->sslErrors();
		    throw ex;
		}
	    }
	}

	//
	// Establish the cipher list.
	//
	{
	    string ciphers = properties->getProperty(propPrefix + "Ciphers");
	    if(!ciphers.empty())
	    {
		if(!SSL_CTX_set_cipher_list(_ctx, ciphers.c_str()))
		{
		    PluginInitializationException ex(__FILE__, __LINE__);
		    ex.reason = "IceSSL: unable to set ciphers using `" + ciphers + "':\n" + _instance->sslErrors();
		    throw ex;
		}
	    }
	}

	//
	// Establish the maximum verify depth.
	//
	{
	    int depth = properties->getPropertyAsIntWithDefault(propPrefix + "VerifyDepthMax", 2);
	    if(depth >= 0)
	    {
		SSL_CTX_set_verify_depth(_ctx, depth);
	    }
	}

	//
	// Diffie Hellman configuration.
	//
	{
#ifndef OPENSSL_NO_DH
	    _dhParams = new DHParams;
	    SSL_CTX_set_options(_ctx, SSL_OP_SINGLE_DH_USE);
	    SSL_CTX_set_tmp_dh_callback(_ctx, opensslDHCallback);
#endif
	    //
	    // Properties have the following form:
	    //
	    // ...DH.<keyLength>=file
	    //
	    const string dhPrefix = propPrefix + "DH.";
	    PropertyDict d = properties->getPropertiesForPrefix(dhPrefix);
	    if(!d.empty())
	    {
#ifdef OPENSSL_NO_DH
		_logger->warning("IceSSL: OpenSSL is not configured for Diffie Hellman");
#else
		for(PropertyDict::iterator p = d.begin(); p != d.end(); ++p)
		{
		    string s = p->first.substr(dhPrefix.size());
		    int keyLength = atoi(s.c_str());
		    if(keyLength > 0)
		    {
			string file = p->second;
			if(!checkPath(file, _defaultDir, false))
			{
			    PluginInitializationException ex(__FILE__, __LINE__);
			    ex.reason = "IceSSL: DH parameter file not found:\n" + file;
			    throw ex;
			}
			if(!_dhParams->add(keyLength, file))
			{
			    PluginInitializationException ex(__FILE__, __LINE__);
			    ex.reason = "IceSSL: unable to read DH parameter file " + file;
			    throw ex;
			}
		    }
		}
#endif
	    }
	}
    }
    catch(...)
    {
	SSL_CTX_free(_ctx);
	throw;
    }
}

IceSSL::Context::~Context()
{
    if(_ctx)
    {
	SSL_CTX_free(_ctx);
    }
}

SSL_CTX*
IceSSL::Context::ctx() const
{
    return _ctx;
}

void
IceSSL::Context::verifyPeer(SSL* ssl, const string& address, bool incoming)
{
    long result = SSL_get_verify_result(ssl);
    if(result != X509_V_OK)
    {
	ostringstream ostr;
	ostr << "IceSSL: certificate verification failed:\n" << X509_verify_cert_error_string(result);
	string msg = ostr.str();
	if(_instance->securityTraceLevel() >= 1)
	{
	    _logger->trace(_instance->securityTraceCategory(), msg);
	}
	SecurityException ex(__FILE__, __LINE__);
	ex.reason = msg;
	throw ex;
    }

    X509* cert = SSL_get_peer_certificate(ssl);
    try
    {
	//
	// Collect the dnsName and ipAddress values that appear in the peer's subjectAltName
        // certificate extension.
	//
	vector<string> dnsNames, ipAddresses;
	if(cert)
	{
	    GENERAL_NAMES* gens = reinterpret_cast<GENERAL_NAMES*>(X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0));
	    int i;
	    for(i = 0; i < sk_GENERAL_NAME_num(gens); ++i)
	    {
		GENERAL_NAME* gen = sk_GENERAL_NAME_value(gens, i);
		if(gen->type == GEN_DNS)
		{
		    ASN1_IA5STRING* str = gen->d.dNSName;
		    if(str && str->type == V_ASN1_IA5STRING && str->data && str->length > 0)
		    {
			string s = reinterpret_cast<const char*>(str->data);
			dnsNames.push_back(s);
		    }
		}
		else if(gen->type == GEN_IPADD)
		{
		    ASN1_OCTET_STRING* addr = gen->d.iPAddress;
		    // TODO: Support IPv6 someday.
		    if(addr && addr->type == V_ASN1_OCTET_STRING && addr->data && addr->length == 4)
		    {
			ostringstream ostr;
			for(int j = 0; j < 4; ++j)
			{
			    if(j > 0)
			    {
				ostr << '.';
			    }
			    ostr << static_cast<int>(addr->data[j]);
			}
			ipAddresses.push_back(ostr.str());
		    }
		}
	    }
	    sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
	}

	CertificateVerifierPtr verifier = _instance->certificateVerifier();

	//
	// Compare the peer's address against the dnsName and ipAddress values.
	// This is only relevant for an outgoing connection.
	//
	if(!address.empty())
	{
	    bool certNameOK = false;

	    for(vector<string>::iterator p = ipAddresses.begin(); p != ipAddresses.end() && !certNameOK; ++p)
	    {
		if(address == *p)
		{
		    certNameOK = true;
		}
	    }

	    if(!certNameOK && !dnsNames.empty())
	    {
		string host = address;
		transform(host.begin(), host.end(), host.begin(), ::tolower);
		for(vector<string>::iterator p = dnsNames.begin(); p != dnsNames.end() && !certNameOK; ++p)
		{
		    string s = *p;
		    transform(s.begin(), s.end(), s.begin(), ::tolower);
		    if(host == s)
		    {
			certNameOK = true;
		    }
		}
	    }

	    //
	    // Log a message if the name comparison fails. If CheckCertName is defined,
	    // we also raise an exception to abort the connection. Don't log a message
	    // if CheckCertName is not defined and a verifier is present.
	    //
	    if(!certNameOK && (_checkCertName || (_instance->securityTraceLevel() >= 1 && !verifier)))
	    {
		ostringstream ostr;
		ostr << "IceSSL: ";
		if(!_checkCertName)
		{
		    ostr << "ignoring ";
		}
		ostr << "certificate validation failure:\npeer certificate does not contain `"
		     << address << "' in its subjectAltName extension";
		if(!dnsNames.empty())
		{
		    ostr << "\nDNS names found in certificate: ";
		    for(vector<string>::iterator p = dnsNames.begin(); p != dnsNames.end(); ++p)
		    {
			if(p != dnsNames.begin())
			{
			    ostr << ", ";
			}
			ostr << *p;
		    }
		}
		if(!ipAddresses.empty())
		{
		    ostr << "\nIP addresses found in certificate: ";
		    for(vector<string>::iterator p = ipAddresses.begin(); p != ipAddresses.end(); ++p)
		    {
			if(p != ipAddresses.begin())
			{
			    ostr << ", ";
			}
			ostr << *p;
		    }
		}
		string msg = ostr.str();
		if(_instance->securityTraceLevel() >= 1)
		{
		    Trace out(_logger, _instance->securityTraceCategory());
		    out << msg;
		}
		if(_checkCertName)
		{
		    SecurityException ex(__FILE__, __LINE__);
		    ex.reason = msg;
		    throw ex;
		}
	    }
	}

	if(verifier)
	{
	    VerifyInfo info;
	    const_cast<bool&>(info.incoming) = incoming;
	    info.cert = cert;
	    info.ssl = ssl;
	    const_cast<string&>(info.address) = address;
	    const_cast<vector<string>&>(info.dnsNames) = dnsNames;
	    const_cast<vector<string>&>(info.ipAddresses) = ipAddresses;
	    if(!verifier->verify(info))
	    {
		string msg = string(incoming ? "incoming" : "outgoing") +
		    " connection rejected by certificate verifier";
		if(_instance->securityTraceLevel() >= 1)
		{
		    _logger->trace(_instance->securityTraceCategory(), msg + "\n" +
				   IceInternal::fdToString(SSL_get_fd(ssl)));
		}
		SecurityException ex(__FILE__, __LINE__);
		ex.reason = msg;
		throw ex;
	    }
	}
    }
    catch(...)
    {
	if(cert)
	{
	    X509_free(cert);
	}
	throw;
    }
    if(cert)
    {
	X509_free(cert);
    }
}

string
IceSSL::Context::password(bool /*encrypting*/)
{
    PasswordPromptPtr prompt = _instance->passwordPrompt();
    if(prompt)
    {
	try
	{
	    return prompt->getPassword();
	}
	catch(...)
	{
	    //
	    // Don't allow exceptions to cross an OpenSSL boundary.
	    //
	    return string();
	}
    }
    else
    {
	return _password;
    }
}

#ifndef OPENSSL_NO_DH
DH*
IceSSL::Context::dhParams(int keyLength)
{
    return _dhParams->get(keyLength);
}
#endif

int
IceSSL::Context::verifyCallback(int ok, SSL* ssl, X509_STORE_CTX* c)
{
    if(!ok && _instance->securityTraceLevel() >= 1)
    {
	X509* cert = X509_STORE_CTX_get_current_cert(c);
	int err = X509_STORE_CTX_get_error(c);
	char buf[256];

	Trace out(_logger, _instance->securityTraceCategory());
	out << "certificate verification failure\n";

	X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
	out << "issuer = " << buf << '\n';
	X509_NAME_oneline(X509_get_subject_name(cert), buf, sizeof(buf));
	out << "subject = " << buf << '\n';
	out << "depth = " << X509_STORE_CTX_get_error_depth(c) << '\n';
	out << "error = " << X509_verify_cert_error_string(err) << '\n';
	out << IceInternal::fdToString(SSL_get_fd(ssl));
    }
    return ok;
}

void
IceSSL::Context::traceConnection(SSL* ssl, bool incoming)
{
    Trace out(_logger, _instance->securityTraceCategory());
    out << "SSL summary for " << (incoming ? "incoming" : "outgoing") << " connection\n";
    SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
    if(!cipher)
    {
	out << "unknown cipher\n";
    }
    else
    {
	out << "cipher = " << SSL_CIPHER_get_name(cipher) << "\n";
	out << "bits = " << SSL_CIPHER_get_bits(cipher, 0) << "\n";
	out << "protocol = " << SSL_get_version(ssl) << "\n";
    }
    out << IceInternal::fdToString(SSL_get_fd(ssl));
}

void
IceSSL::Context::parseProtocols(const string& val)
{
    const string delim = ", ";
    bool sslv3 = false, tlsv1 = false;
    string::size_type pos = 0;
    while(pos != string::npos)
    {
	pos = val.find_first_not_of(delim, pos);
	if(pos == string::npos)
	{
	    break;
	}

	string prot;
	string::size_type end = val.find_first_of(delim, pos);
	if(end == string::npos)
	{
	    prot = val.substr(pos);
	}
	else
	{
	    prot = val.substr(pos, end - pos);
	}
	pos = end;

	if(prot == "ssl3" || prot == "sslv3")
	{
	    sslv3 = true;
	}
	else if(prot == "tls" || prot == "tls1" || prot == "tlsv1")
	{
	    tlsv1 = true;
	}
	else
	{
	    PluginInitializationException ex(__FILE__, __LINE__);
	    ex.reason = "IceSSL: unrecognized protocol `" + prot + "'";
	    throw ex;
	}
    }

    long opts = SSL_OP_NO_SSLv2; // SSLv2 is not supported.
    if(!sslv3)
    {
	opts |= SSL_OP_NO_SSLv3;
    }
    if(!tlsv1)
    {
	opts |= SSL_OP_NO_TLSv1;
    }
    SSL_CTX_set_options(_ctx, opts);
}
