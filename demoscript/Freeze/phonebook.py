#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
import Expect

def dequote(s):
    cur = 0
    end = len(s)
    havequote = False
    quote = None
    items = []
    while cur < len(s):
        if not quote:
            if s[cur] == "'" or s[cur] == '"':
                quote = s[cur]
                start = cur+1
        else:
            if s[cur] == quote:
                items.append(s[start:cur])
                quote = None
        cur = cur + 1
    return items

def mkregexp(s):
    s = s.replace('(', '\\(')
    s = s.replace(')', '\\)')
    s = s.replace('.', '\\.')
    s = s.replace('+', '\\+')
    s = s.replace('*', '\\*')
    return s
                
def run(client, server):
    sys.stdout.write("populating database... ")
    sys.stdout.flush()
    f = open("contacts", "r")
    for l in f:
        client.sendline(l)
        try:
            client.expect('\n', timeout=0)
        except Expect.TIMEOUT:
            pass
    try:
        while True:
            client.expect('\n', timeout=1)
    except Expect.TIMEOUT:
        pass
    print("ok")

    sys.stdout.write("testing... ")
    sys.stdout.flush()
    client.sendline('find "Doe, John"')
    client.expect('number of contacts found: 3')
    client.sendline('next')
    client.expect('current contact is')
    client.sendline('next')
    client.expect('current contact is')
    client.sendline('next')
    client.expect('no current contact')
    client.sendline('find "Mustermann, Manfred"')
    client.expect('number of contacts found: 1')
    client.sendline('next')
    client.expect('no current contact')
    client.sendline('find "Mustermann, Manfred"')
    client.expect('number of contacts found: 1')
    client.sendline('address "foo bar"')
    client.expect('changed address')
    client.sendline('current')
    client.expect('address: foo bar')
    client.sendline('name foo')
    client.expect('changed name')
    client.sendline('current')
    client.expect('name: foo')
    client.sendline('find "Mustermann, Manfred"')
    client.expect('number of contacts found: 0')
    client.sendline('find foo')
    client.expect('number of contacts found: 1')
    client.sendline('remove')
    client.expect('removed current contact')
    client.sendline('remove')
    client.expect('current contact no longer exists')
    client.sendline('find foo')
    client.expect('number of contacts found: 0')

    client.sendline('shutdown')
    if client != server:
        server.waitTestSuccess()
    client.sendline('exit')
    client.waitTestSuccess()

    print("ok")
