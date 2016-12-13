# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class ConfigurationTestCase(ClientServerTestCase):

    def setupServerSide(self, current):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        certsPath = os.path.abspath(os.path.join(self.getPath(), "..", "certs"))
        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            keychainPath = os.path.join(certsPath, "Find.keychain")
            os.system("mkdir -p {0}".format(os.path.join(certsPath, "keychain")))
            os.system("security create-keychain -p password %s" % keychainPath)
            for cert in ["s_rsa_ca1.p12", "c_rsa_ca1.p12"]:
                os.system("security import %s -f pkcs12 -A -P password -k %s" % (os.path.join(certsPath, cert), keychainPath))
        elif platform.hasOpenSSL():
            #
            # Create copies of the CA certificates named after the subject
            # hash. This is used by the tests to find the CA certificates in
            # the IceSSL.DefaultDir
            #
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                os.system("cp {pem} {dir}/`openssl x509 -subject_hash -noout -in {pem}`.0".format(pem=pem, dir=certsPath))

    def teardownServerSide(self, current, success):
        # Nothing to do if we're not running this test with the C++ mapping
        if not isinstance(self.getMapping(), CppMapping):
            return

        certsPath = os.path.abspath(os.path.join(self.getPath(), "..", "certs"))
        if isinstance(platform, Darwin) and current.config.buildPlatform == "macosx":
            os.system("rm -rf {0} {1}".format(os.path.join(certsPath, "keychain"), os.path.join(certsPath, "Find.keychain")))
        elif platform.hasOpenSSL():
            for c in ["cacert1.pem", "cacert2.pem"]:
                pem = os.path.join(certsPath, c)
                os.system("rm -f {dir}/`openssl x509 -subject_hash -noout -in {pem}`.0".format(pem=pem, dir=certsPath))

# Filter-out the deprecated property warnings
outfilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.KeyFile\n", "", x) ]

TestSuite(__name__, [
   ConfigurationTestCase(client=Client(outfilters=outfilters, args=['"{testdir}"']),
                         server=Server(outfilters=outfilters))
], multihost=False)
