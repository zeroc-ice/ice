#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, threading, subprocess, getopt, signal

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]

if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

def removeTrustSettings():
    serverCert = os.path.join(path[0], "certs", "server.pem")
    if os.system("security verify-cert -c " + serverCert + " >& /dev/null") == 0:
        sys.stdout.write("removing trust settings for the HTTP server certificate... ")
        sys.stdout.flush()
        if os.system("security remove-trusted-cert " + serverCert) != 0:
            print("\nerror: couldn't remove trust settings for the HTTP server certificate")
        else:
            print("ok")
    else:
        print("trust settings already removed")

#
# On macOS, provide an option to allow removing the trust settings
#
if TestUtil.isDarwin():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "", ["clean"])
        if ("--clean", "") in opts:
            removeTrustSettings()
            sys.exit(0)
    except getopt.GetoptError:
        pass

iceVersion = "3.6.4"
jar = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..",
                   "java/test/controller/build/libs/testController-%(iceVersion)s.jar" % {"iceVersion": iceVersion})

javaHome = os.environ.get("JAVA_HOME", "")
javaCmd = '%s' % os.path.join(javaHome, "bin", "java") if javaHome else "java"
command = [javaCmd, "-jar", jar]
if len(sys.argv) > 1:
    command += sys.argv[1:]

p = subprocess.Popen(command, shell = False, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
                     stderr = subprocess.STDOUT, bufsize = 0)

def signal_handler(signal, frame):
    if p:
        p.terminate()
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

if TestUtil.isDarwin():
    #
    # On macOS, we set the trust settings on the certificate to prevent
    # the Web browsers from prompting the user about the unstrusted
    # certificate. Some browsers such as Chrome don't provide the
    # option to set this trust settings.
    #
    serverCert = os.path.join(TestUtil.toplevel, "certs", "server.pem")
    if os.system("security verify-cert -c " + serverCert + " >& /dev/null") != 0:
        sys.stdout.write("adding trust settings for the HTTP server certificate... ")
        sys.stdout.flush()
        if os.system("security add-trusted-cert -r trustAsRoot " + serverCert) != 0:
            print("error: couldn't add trust settings for the HTTP server certificate")
        print("ok")
        print("run " + sys.argv[0] + " --clean to remove the trust setting")

while(True):

    c = p.stdout.read(1)
    if not c: break
    if c == '\r': continue

    # Depending on Python version and platform, the value c could be a
    # string or a bytes object.
    if type(c) != str:
        c = c.decode()
    sys.stdout.write(c)
    sys.stdout.flush()
