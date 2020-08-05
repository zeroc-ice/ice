#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, getopt, passlib.hash, passlib.hosts, getpass

usePBKDF2 = any(sys.platform == p for p in ["win32", "darwin", "cygwin"])
useCryptExt = any(sys.platform.startswith(p) for p in ["linux", "freebsd", "gnukfreebsd"])

def usage():
    print("Usage: icehashpassword [options]")
    print("")
    print("OPTIONS")

    if usePBKDF2:
        print("")
        print("  -d MESSAGE_DIGEST_ALGORITHM, --digest=MESSAGE_DIGEST_ALGORITHM")
        print("      The message digest algorithm to use with PBKDF2, valid values are (sha1, sha256, sha512).")
        print("")
        print("  -s SALT_SIZE, --salt=SALT_SIZE")
        print("      Optional number of bytes to use when generating new salts.")
        print("")
    elif useCryptExt:
        print("  -d MESSAGE_DIGEST_ALGORITHM, --digest=MESSAGE_DIGEST_ALGORITHM")
        print("      The message digest algorithm to use with crypt function, valid values are (sha256, sha512).")
        print("")
    if usePBKDF2 or useCryptExt:
        print("  -r ROUNDS, --rounds=ROUNDS")
        print("      Optional number of rounds to use.")
        print("")
    print("  -h, --help" )
    print("      Show this message.")
    print("")

def main():

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
        digestAlgorithms = ("sha256", "sha512")

    try:
        opts, args = getopt.getopt(sys.argv[1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print("")
        print(str(err))
        usage()
        return 2

    digest = None
    salt = None
    rounds = None

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            return 0
        elif o in ("-d", "--digest"):
            if a in digestAlgorithms:
                digest = a
            else:
                print("Unknown digest algorithm `" + a + "'")
                return 2
        elif o in ("-s", "--salt"):
            try:
                salt = int(a)
            except ValueError as err:
                print("Invalid salt size. Value must be an integer")
                usage()
                return 2
        elif o in ("-r", "--rounds"):
            try:
                rounds = int(a)
            except ValueError as err:
                print("Invalid number of rounds. Value must be an integer")
                usage()
                return 2

    passScheme = None
    if usePBKDF2:
        passScheme = passlib.hash.pbkdf2_sha256
        if digest == "sha1":
            passScheme = passlib.hash.pbkdf2_sha1
        elif digest == "sha512":
            passScheme = passlib.hash.pbkdf2_sha512
    elif useCryptExt:
        passScheme = passlib.hash.sha512_crypt
        if digest == "sha256":
            passScheme = passlib.hash.sha256_crypt
    else:
        #
        # Fallback is the OS crypt function
        #
        passScheme = passlib.hosts.host_context

    if rounds:
        if not passScheme.min_rounds <= rounds <= passScheme.max_rounds:
            print("Invalid number rounds for the digest algorithm. Value must be an integer between %s and %s" %
                (passScheme.min_rounds, passScheme.max_rounds))
            usage()
            return 2
    if salt:
        if not passScheme.min_salt_size <= salt <= passScheme.max_salt_size:
            print("Invalid salt size for the digest algorithm. Value must be an integer between %s and %s" %
                (passScheme.min_salt_size, passScheme.max_salt_size))
            usage()
            return 2

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

    # passlib 1.7 renamed encrypt to hash
    if hasattr(passScheme, "hash"):
        print(passScheme.using(**opts).hash(*args))
    else:
        print(passScheme.encrypt(*args, **opts))

    return 0

if __name__ == '__main__':
    sys.exit(main())
