#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
if sys.platform == "win32":
    print "demoscript only supports cygwin python under Windows (use /usr/bin/python expect.py)"
    sys.exit(1)

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

#
# The test language.
#
defaultLanguage = None

import getopt, os, signal
import demoscript.pexpect as pexpect

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
        if mode != 'debug' and mode != 'release':
            usage()

if host != "":
    defaultHost = " --Ice.Default.Host=%s" % (host)
else:
    defaultHost = None

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isDarwin():
   return sys.platform == "darwin"

def isMono():
    return not isCygwin()

def python():
    if isCygwin():
        return "python -u "
    else:
        return "python "

def getIceBox():
    if isCygwin():
        if mode == 'release':
            return "icebox"
        else:
            return "iceboxd"
    return "icebox"

# Automatically adds default host, and uses our default timeout for
# expect.
class spawn(pexpect.spawn):
    def __init__(self, command, language = None):
        if defaultHost:
            command = '%s %s' % (command, defaultHost)
        if debug:
            print '(%s)' % (command)
        if not language:
            self.language = defaultLanguage
        else:
            self.language = language
        self.expectFirst = True
        if trace:
            logfile = sys.stdout
        else:
            logfile = None
        self.sentKill = None
        if self.language == "C#":
            if isMono():
                command = "mono " + command
            else:
                command = "./" + command
        if self.language == "Python":
            command = python() + command
        if self.language == "VB":
            command = "./" + command
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

    def kill(self, sig):
        if isCygwin():
            sig = signal.SIGTERM
        self.sentKill = sig
        return pexpect.spawn.kill(self, sig)
    
    # status == 0 is normal exit status for C++
    #
    # status == 130 is normal exit status for a Java app that was
    # SIGINT interrupted.
    #
    # signalstatus == SIGINT is normal exit status for a mono app,
    # or if under cygwin (since cygwin spawned expect apps cannot
    # catch SIGINT).
    #
    def waitTestSuccess(self, exitstatus = 0, timeout = None):
        if not timeout:
            self.expect(pexpect.EOF)
        else:
            self.expect(pexpect.EOF, timeout)
        status = self.wait()
        if self.language == "C++" or self.language == "Python" or self.language == "Ruby" or self.language == "PHP" or self.language == "VB":
            if isCygwin() and self.sentKill:
                assert self.signalstatus == self.sentKill
            else:
                assert status == exitstatus
        elif self.language == "C#":
            if isMono() or isCygwin() and self.sentKill:
                assert self.signalstatus == self.sentKill
            else:
                assert status == exitstatus
        elif self.language == "Java":
            if self.sentKill:
                if isCygwin():
                    assert self.signalstatus == self.sentKill
                else:
                    if self.sentKill == signal.SIGINT:
                        assert status == 130
                    else:
                        assert False
            else:
                assert status == exitstatus
        else:
            # Unknown language
            print "Warning: unknown language"
            if not self.sentKill:
                assert status == exitstatus
            else:
                assert status == exitstatus or status == 130 or self.signalstatus == self.sentKill

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore" and f != "DB_CONFIG"]:
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
