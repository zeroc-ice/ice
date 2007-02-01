#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# This script converts an XML configuration file for earlier versions of
# the IceSSL plugin to the new property format in 3.1.
#
# Usage:
#
# python convertssl.py myconfig.xml
#
# The equivalent properties are printed to standard output. If any
# compatibility issues are detected, a NOTE comment is emitted.
#

import sys, xml.dom, xml.dom.minidom

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " xmlfile"
    print
    print "Options:"
    print "-h    Show this message."

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    if sys.platform[:6] == "cygwin":
        return 1
    else:
        return 0

def isWin32():
    if sys.platform == "win32" or isCygwin():
        return 1
    else:
        return 0

if isWin32():
    sep = ";"
else:
    sep = ":"

def findChild(parent, name):
    for i in parent.childNodes:
        if i.localName == name:
            return i
    return None

def printConfig(node, name, comment=""):
    prefix = comment + "IceSSL."
    result = "# These properties were converted from the " + name + " configuration.\n"
    result = result + "#\n# NOTE: You may need to define IceSSL.DefaultDir\n"
    general = findChild(node, "general")
    if general:
        if general.attributes.has_key("version"):
            version = general.attributes["version"].nodeValue
            if version == "SSLv3":
                result = result + prefix + "Protocols=SSLv3\n"
            elif version == "TLSv1":
                result = result + prefix + "Protocols=TLSv1\n"
            elif version != "SSLv23":
                print "unknown value `" + version + "' for version attribute"
                sys.exit(1)

        if general.attributes.has_key("cipherlist"):
            result = result + prefix + "Ciphers=" + general.attributes["cipherlist"].nodeValue + "\n"

        if general.attributes.has_key("verifymode"):
            verifymode = general.attributes["verifymode"].nodeValue
            if verifymode == "none":
                result = result + prefix + "VerifyPeer=0\n"
            elif verifymode == "peer":
                result = result + prefix + "VerifyPeer=1\n"
            elif verifymode.find("fail") != -1:
                result = result + prefix + "VerifyPeer=2\n"
            elif verifymode.find("client_once") != -1:
                result = result + prefix + "VerifyPeer=2\n"
            else:
                print "unknown value `" + verifymode + "' for verifymode attribute"
                sys.exit(1)

        if general.attributes.has_key("verifydepth"):
            result = result + prefix + "VerifyDepthMax=" + general.attributes["verifydepth"].nodeValue + "\n"

        if general.attributes.has_key("randombytes"):
            result = result + "# NOTE: You may need to use IceSSL.EntropyDaemon\n"
            result = result + prefix + "Random=" + general.attributes["randombytes"].nodeValue + "\n"

    ca = findChild(node, "certauthority")
    if ca:
        if ca.attributes.has_key("file"):
            result = result + prefix + "CertAuthFile=" + ca.attributes["file"].nodeValue + "\n"
        if ca.attributes.has_key("path"):
            result = result + prefix + "CertAuthDir=" + ca.attributes["path"].nodeValue + "\n"

    basecerts = findChild(node, "basecerts")
    if basecerts:
        certFile = ""
        keyFile = ""
        rsacert = findChild(basecerts, "rsacert")
        if rsacert:
            pub = findChild(rsacert, "public")
            if pub.attributes.has_key("encoding"):
                if pub.attributes["encoding"].nodeValue != "PEM":
                    result = result + "# NOTE: Only PEM encoding is supported for certificates!\n"
            if pub.attributes.has_key("filename"):
                certFile = pub.attributes["filename"].nodeValue
            priv = findChild(rsacert, "private")
            if priv.attributes.has_key("encoding"):
                if priv.attributes["encoding"].nodeValue != "PEM":
                    result = result + "# NOTE: Only PEM encoding is supported for private keys!\n"
            if priv.attributes.has_key("filename"):
                keyFile = priv.attributes["filename"].nodeValue
        dsacert = findChild(basecerts, "dsacert")
        if dsacert:
            pub = findChild(dsacert, "public")
            if pub.attributes.has_key("encoding"):
                if pub.attributes["encoding"].nodeValue != "PEM":
                    result = result + "# NOTE: Only PEM encoding is supported for certificates!\n"
            if pub.attributes.has_key("filename"):
                if len(certFile) > 0:
                    certFile = certFile + sep + pub.attributes["filename"].nodeValue
                else:
                    certFile = pub.attributes["filename"].nodeValue
            priv = findChild(rsacert, "private")
            if priv.attributes.has_key("encoding"):
                if priv.attributes["encoding"].nodeValue != "PEM":
                    result = result + "# NOTE: Only PEM encoding is supported for private keys!\n"
            if priv.attributes.has_key("filename"):
                if len(keyFile) > 0:
                    keyFile = keyFile + sep + priv.attributes["filename"].nodeValue
                else:
                    keyFile = priv.attributes["filename"].nodeValue
        if len(certFile) > 0:
            result = result + prefix + "CertFile=" + certFile + "\n"
        if len(keyFile) > 0:
            result = result + prefix + "KeyFile=" + keyFile + "\n"

        for child in basecerts.childNodes:
            if child.localName == "dhparams":
                keysize = child.attributes["keysize"].nodeValue
                if child.attributes.has_key("encoding"):
                    if child.attributes["encoding"].nodeValue != "PEM":
                        result = result + "# NOTE: Only PEM encoding is supported for DH parameters!\n"
                filename = child.attributes["filename"].nodeValue
                result = result + prefix + "DH." + keysize + "=" + filename + "\n"

    return result

#
# Check arguments
#
xmlfile = None
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        if xmlfile:
            usage()
            sys.exit(1)
        xmlfile = x

if not xmlfile:
    usage()
    sys.exit(1)

f = open(xmlfile, 'r')
doc = xml.dom.minidom.parse(f)
f.close()

config = findChild(doc, "SSLConfig")
if not config:
    print sys.argv[0] + ": unable to find element SSLConfig"
    sys.exit(1)

client = findChild(config, "client")
server = findChild(config, "server")
output = None
if client and server:
    print printConfig(client, "Client")
    print printConfig(server, "Server", "#")
elif client:
    print printConfig(client, "Client")
elif server:
    print printConfig(server, "Server")
