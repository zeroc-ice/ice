#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, time
import demoscript.pexpect as pexpect

def run(createCmd, recreateCmd, readCmd, readnewCmd):
    print "cleaning databases...",
    sys.stdout.flush()
    demoscript.Util.cleanDbDir("db")
    demoscript.Util.cleanDbDir("dbnew")
    print "ok"

    print "creating database...",
    create = demoscript.Util.spawn(createCmd)
    create.expect('7 contacts were successfully created or updated')
    create.waitTestSuccess()
    print "ok"

    print "reading database...",
    read = demoscript.Util.spawn(readCmd)
    read.expect("All contacts \(default order\)")
    read.expect('arnold:\t{1,2}\(333\)333-3333 x1234')
    read.expect('bob:\t{1,2}\(222\)222-2222')
    read.expect('carlos:\t{1,2}\(111\)111-1111')
    read.expect('don:\t{1,2}\(777\)777-7777')
    read.expect('ed:\t{1,2}\(666\)666-6666')
    read.expect('frank:\t{1,2}\(555\)555-5555 x123')
    read.expect('gary:\t{1,2}\(444\)444-4444')
    read.expect('All contacts \(ordered by phone number\)')
    read.expect('carlos:\t{1,2}\(111\)111-1111')
    read.expect('bob:\t{1,2}\(222\)222-2222')
    read.expect('arnold:\t{1,2}\(333\)333-3333 x1234')
    read.expect('gary:\t{1,2}\(444\)444-4444')
    read.expect('frank:\t{1,2}\(555\)555-5555 x123')
    read.expect('ed:\t{1,2}\(666\)666-6666')
    read.expect('don:\t{1,2}\(777\)777-7777')
    read.waitTestSuccess()
    print "ok"

    print "transforming database...",
    transform = demoscript.Util.spawn('transformdb --old ContactData.ice --new NewContactData.ice -f transform.xml  db dbnew', language="C++")
    transform.waitTestSuccess()
    print "ok"

    print "reading new database...",
    readnew = demoscript.Util.spawn(readnewCmd)
    readnew.expect('All contacts \(default order\)')
    readnew.expect('All contacts \(ordered by phone number\)')
    readnew.expect('DbEnv \"dbnew\": Secondary index corrupt: not consistent with primary')
    print "ok"

    print "recreating database...",
    recreate = demoscript.Util.spawn(recreateCmd)
    recreate.expect('Recreated contacts database successfully!')
    recreate.waitTestSuccess()
    print "ok"

    print "rereading new database...",
    readnew = demoscript.Util.spawn(readnewCmd)
    readnew.expect("All contacts \(default order\)")
    readnew.expect('arnold:\t{1,2}\(333\)333-3333 x1234 arnold@gmail.com')
    readnew.expect('bob:\t{1,2}\(222\)222-2222 bob@gmail.com')
    readnew.expect('carlos:\t{1,2}\(111\)111-1111 carlos@gmail.com')
    readnew.expect('don:\t{1,2}\(777\)777-7777 don@gmail.com')
    readnew.expect('ed:\t{1,2}\(666\)666-6666 ed@gmail.com')
    readnew.expect('frank:\t{1,2}\(555\)555-5555 x123 frank@gmail.com')
    readnew.expect('gary:\t{1,2}\(444\)444-4444 gary@gmail.com')
    readnew.expect('All contacts \(ordered by phone number\)')
    readnew.expect('carlos:\t{1,2}\(111\)111-1111 carlos@gmail.com')
    readnew.expect('bob:\t{1,2}\(222\)222-2222 bob@gmail.com')
    readnew.expect('arnold:\t{1,2}\(333\)333-3333 x1234 arnold@gmail.com')
    readnew.expect('gary:\t{1,2}\(444\)444-4444 gary@gmail.com')
    readnew.expect('frank:\t{1,2}\(555\)555-5555 x123 frank@gmail.com')
    readnew.expect('ed:\t{1,2}\(666\)666-6666 ed@gmail.com')
    readnew.expect('don:\t{1,2}\(777\)777-7777 don@gmail.com')
    readnew.waitTestSuccess()
    print "ok"
