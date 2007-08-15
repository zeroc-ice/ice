#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys

def run(client, server):
    print "testing...",
    sys.stdout.flush()
    client.expect('>')
    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/\r{1,2}\n>')

    client.sendline('cd x')
    client.expect('cd x')
    client.expect('`x\': no such directory')
    client.expect('\r{1,2}\n>')

    client.sendline('cd')
    client.expect('cd')
    client.expect('\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/\r{1,2}\n>')

    client.sendline('mkfile a')
    client.expect('mkfile a')
    client.expect('\r{1,2}\n>')

    client.sendline('cd a')
    client.expect('cd a')
    client.expect('`a\': not a directory')
    client.expect('\r{1,2}\n>')

    client.sendline('mkdir a')
    client.expect('mkdir a')
    client.expect('`a\' exists already')
    client.expect('\r{1,2}\n>')

    client.sendline('mkdir b')
    client.expect('mkdir b')
    client.expect('\r{1,2}\n>')

    client.sendline('cd b')
    client.expect('cd b')
    client.expect('\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/b\r{1,2}\n>')

    client.sendline('cd')
    client.expect('cd')
    client.expect('\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/\r{1,2}\n>')

    client.sendline('cd b')
    client.expect('cd b')
    client.expect('\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/b\r{1,2}\n>')

    client.sendline('cd /')
    client.expect('cd /')
    client.expect('\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/\r{1,2}\n>')

    client.sendline('ls')
    client.expect('ls\r{1,2}\n')
    i = client.expect(['a \(file\)', 'b \(directory\)'])
    j = client.expect(['a \(file\)', 'b \(directory\)'])
    assert i != j
    client.expect('\r{1,2}\n>')

    client.sendline('lr')
    client.expect('lr\r{1,2}\n')
    i = client.expect(['a \(file\)', 'b \(directory\)'])
    j = client.expect(['a \(file\)', 'b \(directory\)'])
    assert i != j
    client.expect('\r{1,2}\n>')

    client.sendline('cd b')
    client.expect('cd b\r{1,2}\n>')

    client.sendline('mkdir c')
    client.expect('mkdir c\r{1,2}\n>')

    client.sendline('cd c')
    client.expect('cd c\r{1,2}\n>')

    client.sendline('pwd')
    client.expect('pwd')
    client.expect('/b/c\r{1,2}\n>')

    client.sendline('cd /')
    client.expect('cd /\r{1,2}\n>')

    client.sendline('lr')
    client.expect('lr\r{1,2}\n')
    i = client.expect(['a \(file\)', 'b \(directory\)'])
    if i == 1:
	client.expect('c \(directory\):')
        client.expect('a \(file\)')
    else:
        client.expect('b \(directory\)')
	client.expect('\tc \(directory\):')

    client.sendline('mkfile c')
    client.expect('mkfile c\r{1,2}\n>')

    client.sendline('write c blah c')
    client.expect('write c blah c\r{1,2}\n>')

    client.sendline('cat c')
    client.expect('cat c\r{1,2}\n')
    client.expect('blah\r{1,2}\n')
    client.expect('c')
    client.expect('\r{1,2}\n>')

    client.sendline('rm b')
    client.expect('rm b\r{1,2}\n')
    client.expect('cannot remove `b\': Cannot destroy non-empty directory')
    client.expect('\r{1,2}\n>')

    client.sendline('cd b')
    client.expect('cd b')
    client.expect('\r{1,2}\n>')

    client.sendline('rm *')
    client.expect('rm \*')
    client.expect('\r{1,2}\n>')

    client.sendline('ls')
    client.expect('ls\r{1,2}\n>')

    client.sendline('cd ..')
    client.expect('cd \.\.\r{1,2}\n>')

    client.sendline('rm b')
    client.expect('rm b\r{1,2}\n>')

    client.sendline('rm a c')
    client.expect('rm a c\r{1,2}\n>')

    client.sendline('ls')
    client.expect('ls\r{1,2}\n>')

    client.sendline('exit')
    client.expect(pexpect.EOF)
    assert client.wait() == 0
    print "ok"
