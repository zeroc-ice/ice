#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
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
    " --Ice.SSL.Test.Client.CertPath=TOPLEVELDIR/test/IceSSL/certs"
TestUtil.serverOptions += \
    " --Ice.SSL.Test.Server.CertPath=TOPLEVELDIR/test/IceSSL/certs"
TestUtil.clientServerOptions += \
    " --Ice.SSL.Test.Client.CertPath=TOPLEVELDIR/test/IceSSL/certs" + \
    " --Ice.SSL.Test.Server.CertPath=TOPLEVELDIR/test/IceSSL/certs"

name = os.path.join("IceSSL", "certificateVerification")

print "testing default certificate verifier."
TestUtil.clientServerTest(toplevel, name)

print "testing single-certificate certificate verifier."
TestUtil.clientProtocol += " --Ice.SSL.Client.CertificateVerifier=singleCert"
TestUtil.serverProtocol += " --Ice.SSL.Server.CertificateVerifier=singleCert"
TestUtil.clientServerTest(toplevel, name)

sys.exit(0)
