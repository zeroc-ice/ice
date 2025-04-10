// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import java.net.Socket;
import java.security.Principal;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.X509ExtendedKeyManager;

final class X509KeyManagerI extends X509ExtendedKeyManager {
    X509KeyManagerI(X509ExtendedKeyManager del, String alias, boolean override) {
        _delegate = del;
        _alias = alias;
        _override = override; // Always use the configured alias, don't check for acceptable issuers
    }

    @Override
    public String chooseClientAlias(
            String[] keyType, Principal[] issuers, Socket socket) {
        // Don't bother checking for acceptable issuers if the user configured Ice.SSL.Alias
        if (!_override) {
            String alias = _delegate.chooseClientAlias(keyType, issuers, socket);
            if (alias != null && !alias.isEmpty()) {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String chooseEngineClientAlias(
            String[] keyType, Principal[] issuers, SSLEngine engine) {
        // Don't bother checking for acceptable issuers if the user configured Ice.SSL.Alias
        if (!_override) {
            String alias = _delegate.chooseEngineClientAlias(keyType, issuers, engine);
            if (alias != null && !alias.isEmpty()) {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String chooseServerAlias(
            String keyType, Principal[] issuers, Socket socket) {
        if (!_override) {
            // Don't bother checking for acceptable issuers if the user configured Ice.SSL.Alias.
            String alias = _delegate.chooseServerAlias(keyType, issuers, socket);
            if (alias != null && !alias.isEmpty()) {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public String chooseEngineServerAlias(
            String keyType, Principal[] issuers, SSLEngine engine) {
        // Don't bother checking for acceptable issuers if the user configured Ice.SSL.Alias
        if (!_override) {
            String alias = _delegate.chooseEngineServerAlias(keyType, issuers, engine);
            if (alias != null && !alias.isEmpty()) {
                return alias;
            }
        }
        return _alias;
    }

    @Override
    public X509Certificate[] getCertificateChain(String alias) {
        return _delegate.getCertificateChain(alias);
    }

    @Override
    public String[] getClientAliases(String keyType, Principal[] issuers) {
        return _delegate.getClientAliases(keyType, issuers);
    }

    @Override
    public String[] getServerAliases(String keyType, Principal[] issuers) {
        return _delegate.getServerAliases(keyType, issuers);
    }

    @Override
    public PrivateKey getPrivateKey(String alias) {
        return _delegate.getPrivateKey(alias);
    }

    private final X509ExtendedKeyManager _delegate;
    private final String _alias;
    private final boolean _override;
}
