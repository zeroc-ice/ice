#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, getopt, passlib.hash, getpass

usePBKDF2 = sys.platform == "win32" or sys.platform == "darwin"
useCryptExt = sys.platform.startswith("linux")

def usage():
    print("cryptpasswd [options]")
    print("")
    print("OPTIONS")
    print("")
    if usePBKDF2:
        print("")
        print("  -d DIGEST_ALGORITHM, --digest=DIGEST_ALGORITHM")
        print("      The digest algorithm to use with PBKDF2, valid values are (sha1, sha256, sha512).")
        print("")
        print("  -s SALT_SIZE, --salt=SALT_SIZE")
        print("      Optional number of bytes to use when generating new salts.")
        print("")
    elif useCryptExt:
        print("  -d DIGEST_ALGORITHM, --digest=DIGEST_ALGORITHM")
        print("      The digest algorithm to use with crypt function, valid values are (des, md5, sha256, sha512).")
        print("")
    if usePBKDF2 or useCryptExt:
        print("  -r ROUNDS, --rounds=ROUNDS")
        print("      Optional number of rounds to use.")    
        print("")
    print("  -h, --help" )
    print("      Show this message.")
    print("")

def encrypt():

    digestAlgorithms = ()
    shortArgs = "h"
    longArgs = ["help"]
    if usePBKDF2:
        shortArgs += "d:s:r:"
        longArgs += ["digest=", "salt=", "rounds="]
        digestAlgorithms = ("sha1", "sha256", "sha512")
    elif useCryptExt:
        shortArgs += "d:r:"
        longArgs += ["digest=", "rounds="]
        digestAlgorithms = ("des", "md5", "sha256", "sha512")
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print("")
        print(str(err))
        usage()
        sys.exit(2)

    digest = None
    salt = None
    rounds = None

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit(0)
        elif o in ("-d", "--digest"):                
            if a in digestAlgorithms:
                digest = a
            else:
                print("Unknown digest algorithm `" + a + "'")
                sys.exit(2)
        elif o in ("-s", "--salt"):
            try:
                salt = int(a)
                if salt < 0 or salt > 1024:
                    print("Invalid salt value it must be an integer between 0 and 1024")
                    usage()
                    sys.exit(2) 
            except ValueError as err:
                print("Invalid salt value it must be an integer between 0 and 1024")
                usage()
                sys.exit(2)
        elif o in ("-r", "--rounds"):
            try:
                rounds = int(a)
                if rounds < 1 or rounds > sys.maxsize:
                    print("Invalid rounds value it must be an integer between 1 and %s" % sys.maxsize)
                    usage()
                    sys.exit(2) 
            except ValueError as err:
                print("Invalid rounds value it must be an integer between 1 and %s" % sys.maxsize)
                usage()
                sys.exit(2)

    encryptfn = None
    if usePBKDF2:
        encryptfn = passlib.hash.pbkdf2_sha256.encrypt
        if digest == "sha1":
            encryptfn = passlib.hash.pbkdf2_sha1.encrypt
        elif digest == "sha512":
            encryptfn = passlib.hash.pbkdf2_sha512.encrypt
    elif useCryptExt:
        encryptfn = passlib.hash.sha512_crypt.encrypt
        if digest == "des":
            if rounds:
                print("Custom rounds not allowed with des digest")
                usage()
                sys.exit(2)
            encryptfn = passlib.hash.des_crypt.encrypt
        elif digest == "md5":
            if rounds:
                print("Custom rounds not allowed with md5 digest")
                usage()
                sys.exit(2)
            encryptfn = passlib.hash.md5_crypt.encrypt
        elif digest == "sha256":
            encryptfn = passlib.hash.sha256_crypt.encrypt
    else:
        encryptfn = passlib.hash.des_crypt.encrypt
    
    args = []
    if sys.stdout.isatty():
        args.append(getpass.getpass("Password: "))
    else:
        args.append(sys.stdin.readline().strip())

    opts = {}
    if salt:
        opts["salt_size"] = salt

    if rounds:
        opts["rounds"] = rounds

    print(encryptfn(*args, **opts))

encrypt()
