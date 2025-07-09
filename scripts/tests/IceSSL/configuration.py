# Copyright (c) ZeroC, Inc.

import os
import shutil

from Util import (
    Client,
    ClientServerTestCase,
    CppMapping,
    Darwin,
    Server,
    TestSuite,
    Windows,
    platform,
    run,
)

certsPath = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "certs", "configuration"))


class ConfigurationTestCase(ClientServerTestCase):
    def setupServerSide(self, current):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        self.crlServer = None
        self.ocspServer = None

        if isinstance(platform, Windows) or isinstance(platform, Darwin):
            from scripts.tests.IceSSL import revocationutil

            # Create and start the CRL server for revocation tests.
            self.crlServer = revocationutil.createCRLServer("127.0.0.1", 20001, certsPath)
            self.crlServer.start()

            # Create and start the CRL server for revocation tests.
            self.ocspServer = revocationutil.createOCSPServer("127.0.0.1", 20002, certsPath)
            self.ocspServer.start()

        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            # Create the keychains directory for IceSSL tests.
            keychainsPath = os.path.join(certsPath, "keychain")
            os.makedirs(keychainsPath, exist_ok=True)

            # Create find.keychain for IceSSL.FindCerts tests on macOS
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

    def teardownServerSide(self, current, success):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        if self.crlServer:
            self.crlServer.shutdown()
        if self.ocspServer:
            self.ocspServer.shutdown()

        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            keychainsPath = os.path.join(certsPath, "keychain")
            os.system(f"rm -rf {keychainsPath}")

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
