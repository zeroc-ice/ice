# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class Glacier2HashPasswordTestCase(ClientTestCase):

    def runClientSide(self, current):

        import passlib.hash

        hashpassword = os.path.join(toplevel, "scripts", "icehashpassword.py")

        def test(b):
            if not b:
                raise RuntimeError('test assertion failed')

        def hashPasswords(password, args = ""):
            p = subprocess.Popen('"%s" "%s" %s' % (sys.executable, hashpassword, args), shell=True, stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
            p.stdin.write(password.encode('UTF-8'))
            p.stdin.write('\r\n'.encode('UTF-8'))
            p.stdin.flush()
            if(p.wait() != 0):
                raise RuntimeError("icehashpassword.py failed:\n" + p.stdout.read().decode('UTF-8').strip())
            hash = p.stdout.readline().decode('UTF-8').strip()
            return hash

        usePBKDF2 = sys.platform == "win32" or sys.platform == "darwin"
        useCryptExt = sys.platform.startswith("linux")

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
