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
    f = open("books", "r")
    books = []
    for l in f:
        client.sendline(l)
        client.expect('added new book')
        isbn, title, author = dequote(l)
        books.append((isbn, title, author))
    print("ok")

    byauthor = {}
    for b in books:
        isbn, title, author = b
        if author not in byauthor:
            byauthor[author] = []
        byauthor[author].append(b)

    sys.stdout.write("testing isbn... ")
    sys.stdout.flush()
    for b in books:
        isbn, title, author = b
        client.sendline('isbn %s' %(isbn))
        client.expect('current book is:')
        client.expect('isbn: %s' %(isbn))
        client.expect('title: %s' %(mkregexp(title)))
        client.expect('authors: %s' %(author))
    client.sendline('isbn 1000')
    client.expect('no book with that')
    print("ok")

    sys.stdout.write("testing authors... ")
    sys.stdout.flush()
    for a, bl in byauthor.items():
        client.sendline('authors "%s"' %(a))
        client.expect('number of books found: ([0-9]+)')
        n = int(client.match.group(1))
        assert len(bl) == n
        for i in range(0, n):
            client.expect('current book is:')
            client.expect('isbn: ([a-zA-Z0-9]+)\n')
            findisbn = client.match.group(1)
            nbl = []
            for b in bl:
                isbn, title, author = b
                if not findisbn:
                    nbl.append(b)
                if isbn == findisbn:
                    findisbn = None
                    client.expect('title: %s' %(mkregexp(title)))
                    client.expect('authors: %s' %(author))
            bl = nbl
            assert not findisbn
            client.sendline('next')
        client.expect('no current book')
        assert len(bl) == 0
    client.sendline('authors foo')
    client.expect('number of books found: 0')
    client.expect('no current book')
    print("ok")

    sys.stdout.write("testing rent/return... ")
    sys.stdout.flush()
    isbn, title, author = books[0]
    client.sendline('isbn %s' % (isbn))
    client.expect('current book is:.*isbn.*\ntitle.*\nauthors')
    client.sendline('rent matthew')
    client.expect("the book is now rented by `matthew'")
    client.sendline('current')
    client.expect('rented: matthew')
    client.sendline('rent john')
    client.expect('the book has already been rented')
    client.sendline('return')
    client.expect('the book has been returned')
    client.sendline('current')
    try:
        client.expect('rented:', timeout=2)
    except Expect.TIMEOUT:
        pass
    print("ok")

    sys.stdout.write("testing remove... ")
    sys.stdout.flush()
    isbn, title, author = books[0]
    client.sendline('isbn %s' % (isbn))
    client.expect('current book is:.*isbn.*\ntitle.*\nauthors')
    client.sendline('remove')
    client.expect('removed current book')
    client.sendline('remove')
    client.expect('current book no longer exists')

    client.sendline('shutdown')
    if client != server:
        server.waitTestSuccess()
    client.sendline('exit')
    client.waitTestSuccess()

    print("ok")
