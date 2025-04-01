// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import test.IceSSL.configuration.Test.Server;

class ServerI implements Server {
    ServerI(com.zeroc.Ice.Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public void noCert(com.zeroc.Ice.Current current) {
        try {
            com.zeroc.Ice.SSL.ConnectionInfo info =
                    (com.zeroc.Ice.SSL.ConnectionInfo) current.con.getInfo();
            test(info.certs == null);
        } catch (com.zeroc.Ice.LocalException ex) {
            test(false);
        }
    }

    @Override
    public void checkCert(String subjectDN, String issuerDN, com.zeroc.Ice.Current current) {
        try {
            com.zeroc.Ice.SSL.ConnectionInfo info =
                    (com.zeroc.Ice.SSL.ConnectionInfo) current.con.getInfo();
            java.security.cert.X509Certificate cert =
                    (java.security.cert.X509Certificate) info.certs[0];
            test(info.verified);
            test(
                    info.certs.length == 1 &&
                            cert.getSubjectX500Principal().toString().equals(subjectDN) &&
                            cert.getIssuerX500Principal().toString().equals(issuerDN));
        } catch (com.zeroc.Ice.LocalException ex) {
            test(false);
        }
    }

    @Override
    public void checkCipher(String cipher, com.zeroc.Ice.Current current) {
        try {
            com.zeroc.Ice.SSL.ConnectionInfo info =
                    (com.zeroc.Ice.SSL.ConnectionInfo) current.con.getInfo();
            test(info.cipher.indexOf(cipher) >= 0);
        } catch (com.zeroc.Ice.LocalException ex) {
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

    private com.zeroc.Ice.Communicator _communicator;
}
