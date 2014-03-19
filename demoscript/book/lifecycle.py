#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal

def run(client, server):
    sys.stdout.write("testing... ")
    sys.stdout.flush()
    client.expect('>')
    client.sendline('pwd')
    #client.expect('pwd')
    client.expect('/\n>')

    client.sendline('cd x')
    #client.expect('cd x')
    client.expect('`x\': no such directory')
    client.expect('\n> ')

    client.sendline('cd')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('^/\n> ')

    client.sendline('mkfile a')
    client.expect('^> ')

    client.sendline('cd a')
    client.expect('`a\': not a directory\n> ')

    client.sendline('mkdir a')
    client.expect('`a\' exists already\n> ')

    client.sendline('mkdir b')
    client.expect('^> ')

    client.sendline('cd b')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('^/b\n> ')

    client.sendline('cd')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('^/\n> ')

    client.sendline('cd b')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('^/b\n> ')

    client.sendline('cd /')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('^/\n> ')

    client.sendline('ls')
    client.expectall(['a \(file\)\n', 'b \(directory\)\n'])
    client.expect('^> ')

    client.sendline('lr')
    client.expectall(['a \(file\)\n', 'b \(directory\):\n'])
    client.expect('^> ')

    client.sendline('cd b')
    client.expect('^> ')

    client.sendline('mkdir c')
    client.expect('^> ')

    client.sendline('cd c')
    client.expect('^> ')

    client.sendline('pwd')
    client.expect('/b/c\n> ')

    client.sendline('cd /')
    client.expect('^> ')

    client.sendline('lr')
    client.expectall(['a \(file\)\n', 'b \(directory\):\n\tc \(directory\):\n'])
    client.expect('^> ')

    client.sendline('mkfile c')
    client.expect('^> ')

    client.sendline('write c blah c')
    client.expect('^> ')

    client.sendline('cat c')
    client.expect('blah\n')
    client.expect('c\n')
    client.expect('^> ')

    client.sendline('rm b')
    client.expect('cannot remove `b\': Cannot destroy non-empty directory')
    client.expect('\n> ')

    client.sendline('cd b')
    client.expect('^> ')

    client.sendline('rm *')
    client.expect('^> ')

    client.sendline('ls')
    client.expect('^> ')

    client.sendline('cd ..')
    client.expect('^> ')

    client.sendline('rm b')
    client.expect('^> ')

    client.sendline('rm a c')
    client.expect('^> ')

    client.sendline('ls')
    client.expect('^> ')

    client.sendline('exit')
    client.waitTestSuccess()

    server.kill(signal.SIGINT)
    server.waitTestSuccess()
    print("ok")
