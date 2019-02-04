//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceSSL;

import javax.net.ssl.X509ExtendedKeyManager;

final class X509KeyManagerI extends X509ExtendedKeyManager
{
    X509KeyManagerI(javax.net.ssl.X509ExtendedKeyManager del, String alias, boolean override)
    {
        _delegate = del;
        _alias = alias;
        _override = override; // Always use the configured alias, don't check for acceptable issuers
    }

    @Override
    public String
    chooseClientAlias(String[] keyType, java.security.Principal[] issuers, java.net.Socket socket)
    {
        if(!_override) // Don't bother checking for acceptable issuers if the user configured IceSSL.Alias
        {
            String alias = _delegate.chooseClientAlias(keyType, issuers, socket);
            if(alias != null && !alias.isEmpty())
            {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String
    chooseEngineClientAlias(String[] keyType, java.security.Principal[] issuers, javax.net.ssl.SSLEngine engine)
    {
        if(!_override) // Don't bother checking for acceptable issuers if the user configured IceSSL.Alias
        {
            String alias = _delegate.chooseEngineClientAlias(keyType, issuers, engine);
            if(alias != null && !alias.isEmpty())
            {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String
    chooseServerAlias(String keyType, java.security.Principal[] issuers, java.net.Socket socket)
    {
        if(!_override) // Don't bother checking for acceptable issuers if the user configured IceSSL.Alias
        {
            String alias = _delegate.chooseServerAlias(keyType, issuers, socket);
            if(alias != null && !alias.isEmpty())
            {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String
    chooseEngineServerAlias(String keyType, java.security.Principal[] issuers, javax.net.ssl.SSLEngine engine)
    {
        if(!_override) // Don't bother checking for acceptable issuers if the user configured IceSSL.Alias
        {
            String alias = _delegate.chooseEngineServerAlias(keyType, issuers, engine);
            if(alias != null && !alias.isEmpty())
            {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public java.security.cert.X509Certificate[]
    getCertificateChain(String alias)
    {
        return _delegate.getCertificateChain(alias);
    }

    @Override
    public String[]
    getClientAliases(String keyType, java.security.Principal[] issuers)
    {
        return _delegate.getClientAliases(keyType, issuers);
    }

    @Override
    public String[]
    getServerAliases(String keyType, java.security.Principal[] issuers)
    {
        return _delegate.getServerAliases(keyType, issuers);
    }

    @Override
    public java.security.PrivateKey
    getPrivateKey(String alias)
    {
        return _delegate.getPrivateKey(alias);
    }

    private javax.net.ssl.X509ExtendedKeyManager _delegate;
    private String _alias;
    private boolean _override;
}
