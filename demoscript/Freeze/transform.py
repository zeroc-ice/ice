#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, time
from scripts import Expect

def run(createCmd, recreateCmd, readCmd, readnewCmd):
    sys.stdout.write("cleaning databases... ")
    sys.stdout.flush()
    demoscript.Util.cleanDbDir("db")
    demoscript.Util.cleanDbDir("dbnew")
    print("ok")

    sys.stdout.write("creating database... ")
    sys.stdout.flush()
    create = demoscript.Util.spawn(createCmd)
    create.expect('7 contacts were successfully created or updated')
    create.waitTestSuccess()
    print("ok")

    sys.stdout.write("reading database... ")
    sys.stdout.flush()
    read = demoscript.Util.spawn(readCmd)
    read.expect(["All contacts \(default order\)",
                 'arnold:\t{1,2}\(333\)333-3333 x1234',
                 'bob:\t{1,2}\(222\)222-2222',
                 'carlos:\t{1,2}\(111\)111-1111',
                 'don:\t{1,2}\(777\)777-7777',
                 'ed:\t{1,2}\(666\)666-6666',
                 'frank:\t{1,2}\(555\)555-5555 x123',
                 'gary:\t{1,2}\(444\)444-4444',
                 'All contacts \(ordered by phone number\)',
                 'carlos:\t{1,2}\(111\)111-1111',
                 'bob:\t{1,2}\(222\)222-2222',
                 'arnold:\t{1,2}\(333\)333-3333 x1234',
                 'gary:\t{1,2}\(444\)444-4444',
                 'frank:\t{1,2}\(555\)555-5555 x123',
                 'ed:\t{1,2}\(666\)666-6666',
                 'don:\t{1,2}\(777\)777-7777'])
    read.waitTestSuccess()
    print("ok")

    sys.stdout.write("transforming database... ")
    sys.stdout.flush()
    transform = demoscript.Util.spawn('transformdb --old ContactData.ice --new NewContactData.ice -f transform.xml  db dbnew')
    transform.waitTestSuccess()
    print("ok")

    sys.stdout.write("reading new database... ")
    sys.stdout.flush()
    readnew = demoscript.Util.spawn(readnewCmd)
    readnew.expect('All contacts \(default order\)')
    readnew.expect('All contacts \(ordered by phone number\)')
    readnew.expect('DbEnv \"dbnew\": contacts: .* DB_SECONDARY_BAD: Secondary index inconsistent with primary')
    readnew.waitTestSuccess(1)
    print("ok")

    sys.stdout.write("recreating database... ")
    sys.stdout.flush()
    recreate = demoscript.Util.spawn(recreateCmd)
    recreate.expect('Recreated contacts database successfully!')
    recreate.waitTestSuccess()
    print("ok")

    sys.stdout.write("rereading new database... ")
    sys.stdout.flush()
    readnew = demoscript.Util.spawn(readnewCmd)
    readnew.expect(["All contacts \(default order\)",
                    'arnold:\t{1,2}\(333\)333-3333 x1234 arnold@gmail.com',
                    'bob:\t{1,2}\(222\)222-2222 bob@gmail.com',
                    'carlos:\t{1,2}\(111\)111-1111 carlos@gmail.com',
                    'don:\t{1,2}\(777\)777-7777 don@gmail.com',
                    'ed:\t{1,2}\(666\)666-6666 ed@gmail.com',
                    'frank:\t{1,2}\(555\)555-5555 x123 frank@gmail.com',
                    'gary:\t{1,2}\(444\)444-4444 gary@gmail.com',
                    'All contacts \(ordered by phone number\)',
                    'carlos:\t{1,2}\(111\)111-1111 carlos@gmail.com',
                    'bob:\t{1,2}\(222\)222-2222 bob@gmail.com',
                    'arnold:\t{1,2}\(333\)333-3333 x1234 arnold@gmail.com',
                    'gary:\t{1,2}\(444\)444-4444 gary@gmail.com',
                    'frank:\t{1,2}\(555\)555-5555 x123 frank@gmail.com',
                    'ed:\t{1,2}\(666\)666-6666 ed@gmail.com',
                    'don:\t{1,2}\(777\)777-7777 don@gmail.com',])
    readnew.waitTestSuccess()
    print("ok")
