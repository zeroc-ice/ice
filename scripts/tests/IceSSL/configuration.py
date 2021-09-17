# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os, threading, http.server
from scripts.tests.IceSSL import ocsphandler
from functools import partial

class ConfigurationTestCase(ClientServerTestCase):

    def setupServerSide(self, current):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        certsPath = os.path.abspath(os.path.join(current.testsuite.getPath(), "..", "certs"))

        self.clrServer = None
        self.ocspServer = None

        if isinstance(platform, Windows) or isinstance(platform, Darwin):
            self.crlServer = createCRLServer('127.0.0.1', 20001, certsPath)
            self.crlServer.start()
            self.ocspServer = createOCSPServer('127.0.0.1', 20002, certsPath)
            self.ocspServer.start()

        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            keychainPath = os.path.join(certsPath, "Find.keychain")
            os.system("mkdir -p {0}".format(os.path.join(certsPath, "keychain")))
            os.system("security create-keychain -p password %s" % keychainPath)
            for cert in ["s_rsa_ca1.p12", "c_rsa_ca1.p12"]:
                os.system("security import %s -f pkcs12 -A -P password -k %s" % (os.path.join(certsPath, cert), keychainPath))
        elif current.config.openssl or platform.hasOpenSSL():
            if isinstance(platform, Windows):
                conf = os.path.join(current.testsuite.getPath(), "openssl.cnf")
                os.environ["OPENSSL_CONF"] = conf
                with open(conf, "w") as file:
                    file.write("# Dummy openssl configuration file to avoid warnings with Windows testing")

            #
            # Create copies of the CA certificates named after the subject
            # hash. This is used by the tests to find the CA certificates in
            # the IceSSL.DefaultDir
            #
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                out = run("{openssl} x509 -subject_hash -noout -in {pem}".format(pem=pem, openssl=self.getOpenSSLCommand(current)))
                shutil.copyfile(pem, "{dir}/{out}.0".format(dir=certsPath, out=out))

    def teardownServerSide(self, current, success):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        if self.crlServer:
            self.crlServer.shutdown()
        if self.ocspServer:
            self.ocspServer.shutdown()

        certsPath = os.path.abspath(os.path.join(current.testsuite.getPath(), "..", "certs"))
        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            os.system("rm -rf {0} {1}".format(os.path.join(certsPath, "keychain"), os.path.join(certsPath, "Find.keychain")))
        elif current.config.openssl or platform.hasOpenSSL():
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                out = run("{openssl} x509 -subject_hash -noout -in {pem}".format(pem=pem, openssl=self.getOpenSSLCommand(current)))
                os.remove("{dir}/{out}.0".format(out=out, dir=certsPath))
            if isinstance(platform, Windows):
                os.remove(os.path.join(current.testsuite.getPath(), "openssl.cnf"))
                del os.environ["OPENSSL_CONF"]

    def getOpenSSLCommand(self, current):
        if isinstance(platform, Windows):
            return os.path.join(current.testsuite.getPath(), "..", "..", "..", "msbuild", "packages",
                                "zeroc.openssl.v140.1.1.1", "build", "native", "bin", "Win32", "Release",
                                "openssl.exe")
        else:
            return "openssl"

class IceSSLConfigurationClient(Client):

    def getExe(self, current):
        if isinstance(platform, Windows) and current.config.openssl:
            return "clientopenssl"
        return Client.getExe(self, current)

class IceSSLConfigurationServer(Server):

    def getExe(self, current):
        if isinstance(platform, Windows) and current.config.openssl:
            return "serveropenssl"
        return Server.getExe(self, current)

# Filter-out the deprecated property warnings
outfilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.KeyFile\n", "", x) ]

#
# With UWP, we can't run this test with the UWP C++ server (used with tcp/ws)
#
options=lambda current: { "protocol": ["ssl", "wss"] } if current.config.uwp else {}

TestSuite(__name__, [
   ConfigurationTestCase(client=IceSSLConfigurationClient(outfilters=outfilters, args=['"{testdir}"']),
                         server=IceSSLConfigurationServer(outfilters=outfilters, args=['"{testdir}"']))
], multihost=False, options=options)


def createOCSPServer(host, port, basepath):
    db = ocsphandler.load_db(basepath)
    handler = partial(ocsphandler.OCSPHandler, db)
    return ThreadedServer(host, port, handler)

def createCRLServer(host, port, basepath,):
    handler = partial(http.server.SimpleHTTPRequestHandler, directory=basepath)
    return  ThreadedServer(host, port, handler)


class ThreadedServer:
    # run HTPPServer in its own thread

    def __init__(self, hostname, port, handler):
        self.handler = handler
        self.server = http.server.HTTPServer((hostname, port), handler)
        self.thread = None

    def start(self):
        def serve_forever(server):
            with server:
                server.serve_forever()

        self.thread = threading.Thread(target=serve_forever, args=(self.server,))
        self.thread.setDaemon(True)
        self.thread.start()

    def shutdown(self):
        self.server.shutdown()
        self.thread.join()
