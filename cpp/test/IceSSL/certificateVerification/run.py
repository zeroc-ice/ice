#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

if TestUtil.protocol != "ssl":
    print "This test may only be run with SSL enabled."
    sys.exit(0)

testOptions = " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "test", "IceSSL", "certs") + \
              " --IceSSL.Client.Config= " + \
              " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "test", "IceSSL", "certs") + \
              " --IceSSL.Server.Config= "

TestUtil.clientOptions += " --IceSSL.Test.Client.CertPath=" + \
                          os.path.join(toplevel, "test", "IceSSL", "certs")
TestUtil.serverOptions += " --IceSSL.Test.Server.CertPath=" + \
                          os.path.join(toplevel, "test", "IceSSL", "certs")
TestUtil.clientServerOptions += " --IceSSL.Test.Client.CertPath=" + \
                                os.path.join(toplevel, "test", "IceSSL", "certs") + \
                                " --IceSSL.Test.Server.CertPath=" + \
                                os.path.join(toplevel, "test", "IceSSL", "certs")

name = os.path.join("IceSSL", "certificateVerification")

print "testing default certificate verifier."
TestUtil.clientServerTest(name)

print "testing single-certificate certificate verifier."
TestUtil.clientOptions += " --IceSSL.Client.CertificateVerifier=singleCert"
TestUtil.serverOptions += " --IceSSL.Server.CertificateVerifier=singleCert"
TestUtil.clientServerTest(name)

sys.exit(0)
