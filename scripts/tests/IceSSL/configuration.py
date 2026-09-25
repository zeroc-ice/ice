# Copyright (c) ZeroC, Inc.

import os
import shutil

from Util import (
    Client,
    ClientServerTestCase,
    CppMapping,
    CSharpMapping,
    Darwin,
    Driver,
    Server,
    TestSuite,
    Windows,
    platform,
    run,
)

certsPath = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "certs", "configuration"))


class ConfigurationTestCase(ClientServerTestCase):
    def usesRevocationServers(self) -> bool:
        # The C++ revocation tests fetch CRLs and OCSP responses over HTTP only with Schannel and SecureTransport;
        # OpenSSL reads the CRL files directly. .NET fetches them on every platform.
        mapping = self.getMapping()
        if isinstance(mapping, CSharpMapping):
            return True
        return isinstance(mapping, CppMapping) and (isinstance(platform, Windows) or isinstance(platform, Darwin))

    def setupServerSide(self, current: Driver.Current) -> None:
        self.crlServer = None
        self.ocspServer = None

        if self.usesRevocationServers():
            from scripts.tests.IceSSL import revocationutil

            # Create and start the CRL server for revocation tests.
            self.crlServer = revocationutil.createCRLServer("127.0.0.1", 20001, certsPath)
            self.crlServer.start()

            # Create and start the CRL server for revocation tests.
            self.ocspServer = revocationutil.createOCSPServer("127.0.0.1", 20002, certsPath)
            self.ocspServer.start()

        # The keychain and hashed CA directory setup below is only used by the C++ tests.
        if not isinstance(self.getMapping(), CppMapping):
            return

        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            # Create Find.keychain for IceSSL.FindCert tests on macOS. The default cert-import path no
            # longer needs a pre-created keychain — when IceSSL.Keychain is unset, IceSSL creates a
            # private temporary keychain itself.
            keychainPath = os.path.join(certsPath, "Find.keychain")
            os.system(f"security create-keychain -p password {keychainPath}")
            for cert in ["ca1/server.p12", "ca1/client.p12"]:
                os.system(f"security import {os.path.join(certsPath, cert)} -f pkcs12 -A -P password -k {keychainPath}")
        elif current.config.openssl or platform.hasOpenSSL():
            # Create copies of the CA certificates named after the subject hash. This is used by the tests to find the
            # CA certificates in the IceSSL.DefaultDir.
            for cert in ["ca1/ca1_cert.pem", "ca2/ca2_cert.pem"]:
                certFile = os.path.join(certsPath, cert)
                out = run(f"openssl x509 -subject_hash -noout -in {certFile}")
                shutil.copyfile(certFile, f"{certsPath}/{out}.0")

    def teardownServerSide(self, current: Driver.Current, success: bool) -> None:
        if self.crlServer:
            self.crlServer.shutdown()
        if self.ocspServer:
            self.ocspServer.shutdown()

        if not isinstance(self.getMapping(), CppMapping):
            return

        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            findKeychain = os.path.join(certsPath, "Find.keychain")
            os.system(f"rm -rf {findKeychain}")
        elif current.config.openssl or platform.hasOpenSSL():
            for cert in ["ca1/ca1_cert.pem", "ca2/ca2_cert.pem"]:
                out = run(f"openssl x509 -subject_hash -noout -in {os.path.join(certsPath, cert)}")
                os.remove(f"{certsPath}/{out}.0")


TestSuite(
    __name__,
    [ConfigurationTestCase(client=Client(args=[f'"{certsPath}"']), server=Server(args=[f'"{certsPath}"']))],
    multihost=False,
    options={"protocol": ["tcp"]},
)
