#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, passlib.hash, subprocess

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

hashpassword = os.path.join(path[0], "scripts", "icehashpassword.py")

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def hashPasswords(password, args = ""):
    p = subprocess.Popen('%s "%s" %s' % (sys.executable, hashpassword, args), shell=True, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
    p.stdin.write(password.encode('UTF-8'))
    p.stdin.write('\r\n'.encode('UTF-8'))
    p.stdin.flush()
    if(p.wait() != 0):
        print("icehashpassword.py failed:\n" + p.stdout.read().decode('UTF-8').strip())
        sys.exit(1)
    hash = p.stdout.readline().decode('UTF-8').strip()
    return hash


usePBKDF2 = sys.platform == "win32" or sys.platform == "darwin"
useCryptExt = sys.platform.startswith("linux")

if usePBKDF2:

    sys.stdout.write("Testing PBKDF2 crypt passwords...")
    sys.stdout.flush()

    test(passlib.hash.pbkdf2_sha256.verify("abc123", hashPasswords("abc123")))
    test(not passlib.hash.pbkdf2_sha256.verify("abc123", hashPasswords("abc")))

    test(passlib.hash.pbkdf2_sha1.verify("abc123", hashPasswords("abc123", "-d sha1")))
    test(not passlib.hash.pbkdf2_sha1.verify("abc123", hashPasswords("abc", "-d sha1")))

    test(passlib.hash.pbkdf2_sha512.verify("abc123", hashPasswords("abc123", "-d sha512")))
    test(not passlib.hash.pbkdf2_sha512.verify("abc123", hashPasswords("abc", "-d sha512")))

    #
    # Now use custom rounds
    #
    hash = hashPasswords("abc123", "-r 1000")
    if hash.find("$pbkdf2-sha256$1000$") == -1:
        test(False)
    test(passlib.hash.pbkdf2_sha256.verify("abc123", hash))

    hash = hashPasswords("abc123", "-r 1000 -d sha1")
    if hash.find("$pbkdf2$1000$") == -1:
        test(False)
    test(passlib.hash.pbkdf2_sha1.verify("abc123", hash))

    hash = hashPasswords("abc123", "-r 1000 -d sha512")
    if hash.find("$pbkdf2-sha512$1000$") == -1:
        test(False)
    test(passlib.hash.pbkdf2_sha512.verify("abc123", hash))

    print("ok")

elif useCryptExt:

    sys.stdout.write("Testing Linux crypt passwords...")
    sys.stdout.flush()

    test(passlib.hash.sha512_crypt.verify("abc123", hashPasswords("abc123")))
    test(not passlib.hash.sha512_crypt.verify("abc123", hashPasswords("abc")))

    test(passlib.hash.sha256_crypt.verify("abc123", hashPasswords("abc123", "-d sha256")))
    test(not passlib.hash.sha256_crypt.verify("abc123", hashPasswords("abc", "-d sha256")))

    #
    # Now use custom rounds
    #
    hash = hashPasswords("abc123", "-r 5000")
    if hash.find("rounds=") != -1:
        test(False)
    test(passlib.hash.sha512_crypt.verify("abc123", hash))
    hash = hashPasswords("abc123", "-d sha256 -r 5000")
    if hash.find("rounds=") != -1:
        test(False)
    test(passlib.hash.sha256_crypt.verify("abc123", hash))

    hash = hashPasswords("abc123", "-r 10000")
    if hash.find("$rounds=10000$") == -1:
        test(False)
    test(passlib.hash.sha512_crypt.verify("abc123", hash))
    hash = hashPasswords("abc123", "-d sha256 -r 10000")
    if hash.find("$rounds=10000$") == -1:
        test(False)
    test(passlib.hash.sha256_crypt.verify("abc123", hash))

    print("ok")
