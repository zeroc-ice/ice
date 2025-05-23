// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.SSL.ConnectionInfo;

import test.IceSSL.configuration.Test.Server;

import java.security.cert.X509Certificate;

class ServerI implements Server {
    ServerI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public void noCert(Current current) {
        try {
            ConnectionInfo info =
                (ConnectionInfo) current.con.getInfo();
            test(info.certs == null);
        } catch (LocalException ex) {
            test(false);
        }
    }

    @Override
    public void checkCert(String subjectDN, String issuerDN, Current current) {
        try {
            ConnectionInfo info = (ConnectionInfo) current.con.getInfo();
            X509Certificate cert = (X509Certificate) info.certs[0];
            test(info.verified);
            test(
                info.certs.length == 2
                    && cert.getSubjectX500Principal().toString().equals(subjectDN)
                    && cert.getIssuerX500Principal().toString().equals(issuerDN));
        } catch (LocalException ex) {
            test(false);
        }
    }

    @Override
    public void checkCipher(String cipher, Current current) {
        try {
            ConnectionInfo info =
                (ConnectionInfo) current.con.getInfo();
            test(info.cipher.indexOf(cipher) >= 0);
        } catch (LocalException ex) {
            test(false);
        }
    }

    public void destroy() {
        _communicator.destroy();
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private final Communicator _communicator;
}
