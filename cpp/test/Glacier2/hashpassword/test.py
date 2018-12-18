# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

class Glacier2HashPasswordTestCase(ClientTestCase):

    def runClientSide(self, current):

        import passlib.hash

        hashpassword = os.path.join(toplevel, "scripts", "icehashpassword.py")
        usePBKDF2 = sys.platform == "win32" or sys.platform == "darwin"
        useCryptExt = sys.platform.startswith("linux")

        def test(b):
            if not b:
                raise RuntimeError('test assertion failed')

        def hashPasswords(password, args = ""):
            return run('"%s" "%s" %s' % (sys.executable, hashpassword, args),
                       stdin=(password + "\r\n").encode('UTF-8'),
                       stdinRepeat=False)

        if usePBKDF2:

            current.write("Testing PBKDF2 crypt passwords...")

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

            current.writeln("ok")

        elif useCryptExt:

            current.write("Testing Linux crypt passwords...")

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

            current.writeln("ok")

TestSuite(__name__, [Glacier2HashPasswordTestCase()])
