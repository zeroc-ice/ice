#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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

TestUtil.clientOptions += \
    " --IceSSL.Test.Client.CertPath=TOPLEVELDIR/test/IceSSL/certs"
TestUtil.serverOptions += \
    " --IceSSL.Test.Server.CertPath=TOPLEVELDIR/test/IceSSL/certs"
TestUtil.clientServerOptions += \
    " --IceSSL.Test.Client.CertPath=TOPLEVELDIR/test/IceSSL/certs" + \
    " --IceSSL.Test.Server.CertPath=TOPLEVELDIR/test/IceSSL/certs"

name = os.path.join("IceSSL", "certificateVerification")

print "testing default certificate verifier."
TestUtil.clientServerTest(toplevel, name)

print "testing single-certificate certificate verifier."
TestUtil.clientOptions += " --IceSSL.Client.CertificateVerifier=singleCert"
TestUtil.serverOptions += " --IceSSL.Server.CertificateVerifier=singleCert"
TestUtil.clientServerTest(toplevel, name)

sys.exit(0)
