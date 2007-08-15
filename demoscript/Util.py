# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Timeout after the initial spawn.
#
initialTimeout = 10
#
# Default timeout on subsequent expect calls.
#
defaultTimeout = 5

#
# Default value of --Ice.Default.Host
#
host = "127.0.0.1"

#
# Echo the commands.
#
debug = False

import sys, getopt, pexpect, os

def usage():
    print "usage: " + sys.argv[0] + " --fast --trace --debug --host host --mode=[debug|release]"
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["fast", "trace", "debug", "host=", "mode="])
except getopt.GetoptError:
    usage()

fast = False
trace = False
mode = 'release'
for o, a in opts:
    if o == "--debug":
        debug = True
    if o == "--trace":
        trace = True
    if o == "--host":
        host = a
    if o == "--fast":
        fast = True
    if o == "--mode":
        mode = a
        if mode != 'debug' or mode != 'release':
            print "usage: " + sys.argv[0] + " --trace --debug --host host --mode=[debug|release]"
            sys.exit(2)

if host != "":
    defaultHost = " --Ice.Default.Host=%s" % (host)
else:
    defaultHost = None

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isWin32():
    return sys.platform == "win32" or isCygwin()

def isDarwin():
   return sys.platform == "darwin"

def mono():
    if isWin32():
        return ""
    else:
        return "mono "

def getIceBox():
    if isWin32():
        if mode == 'release':
            return "icebox"
        else:
            return "iceboxd"
    return "icebox"

# Automatically adds default host, and uses our default timeout for
# expect.
class spawn(pexpect.spawn):
    def __init__(self, command):
        if defaultHost:
            command = '%s %s' % (command, defaultHost)
        if debug:
            print '(%s)' % (command)
        self.expectFirst = True
        if trace:
            logfile = sys.stdout
        else:
            logfile = None
        pexpect.spawn.__init__(self, command, logfile = logfile)
    def expect(self, pattern, timeout = defaultTimeout, searchwindowsize=None):
        if self.expectFirst and timeout == defaultTimeout:
            timeout = initialTimeout
            self.expectFirst = False
        return pexpect.spawn.expect(self, pattern, timeout, searchwindowsize)
    def wait(self):
        try:
            return pexpect.spawn.wait(self)
        except pexpect.ExceptionPexpect, e:
            return self.exitstatus

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore"]:
        if os.path.isdir(filename):
            cleanDbDir(filename)
            try:
                os.rmdir(filename)
            except OSError:
                # This might fail if the directory is empty (because
                # it itself contains a .gitignore file.
                pass
        else:
            os.remove(filename)
