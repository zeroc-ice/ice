# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import glob

def test(process, current, match, enc, mapping):
    cmd = process.getCommandLine(current)
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=mapping.getEnv(process, current))
    out, err = p.communicate()
    ret = p.poll()
    if ret != 0:
        raise RuntimeError("%s failed! status %s " % (cmd, ret))
    if out.decode(enc).find(match.decode(enc)) == -1:
        raise RuntimeError("test failed")

class IceLoggerTestCase(ClientTestCase):

    def runClientSide(self, current):

        client1 = Client(exe="client1")
        client2 = Client(exe="client2")
        client3 = Client(exe="client3")
        client4 = Client(exe="client4")
        client5 = Client(exe="client5")

        client1.run(current)

        sys.stdout.write("testing logger ISO-8859-15 output... ")
        test(client2, current, b'aplicaci\xf3n', "ISO-8859-15", self.getMapping())
        print("ok")

        sys.stdout.write("testing logger UTF8 output without string converter... ")
        test(client3, current, b'aplicaci\xc3\xb3n', "UTF8", self.getMapping())
        print("ok")

        sys.stdout.write("testing logger UTF8 output with ISO-8859-15 narrow string converter... ")
        #
        # In Windows expected output is UTF8, because the console output code page is set to UTF-8
        # in Linux and OS X, the expected output is ISO-8859-15 because that is the narrow string
        # encoding used by the application.
        #
        if isinstance(platform, Windows):
            test(client4, current, b'aplicaci\xc3\xb3n', "UTF8", self.getMapping())
        else:
            test(client4, current, b'aplicaci\xf3n', "ISO-8859-15", self.getMapping())
        print("ok")

        sys.stdout.write("testing logger file rotation... ")
        self.clean()

        os.makedirs("log")
        open("log/client5-4.log", 'a').close()

        if isinstance(platform, Windows):
            os.system("echo Y|cacls log /P \"%USERNAME%\":R 1> nul")
        else:
            os.system("chmod -w log")

        client5.run(current)

        if isinstance(platform, Windows):
            os.system("echo Y|cacls log /P \"%USERNAME%\":F 1> nul")
        else:
            os.system("chmod +w log")

        if (not os.path.isfile("client5-0.log") or
            not os.stat("client5-0.log").st_size == 512 or
            len(glob.glob("client5-0-*.log")) != 19):
            raise RuntimeError("failed!")

        for f in glob.glob("client5-0-*.log"):
            if not os.stat(f).st_size == 512:
                print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
                sys.exit(1)

        if (not os.path.isfile("client5-1.log") or
            not os.stat("client5-1.log").st_size == 1024 or
            len(glob.glob("client5-1-*.log")) != 0):
            raise RuntimeError("failed!")

        if (not os.path.isfile("client5-2.log") or
            not os.stat("client5-2.log").st_size == 128 or
            len(glob.glob("client5-2-*.log")) != 7):
            raise RuntimeError("failed!")

        for f in glob.glob("client5-2-*.log"):
            if not os.stat(f).st_size == 128:
                print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
                raise RuntimeError("failed!")

        if (not os.path.isfile("client5-3.log") or
            not os.stat("client5-2.log").st_size == 128 or
            len(glob.glob("client5-2-*.log")) != 7):
            raise RuntimeError("failed!")

        for f in glob.glob("client5-3-*.log"):
            if not os.stat(f).st_size == 128:
                print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
                raise RuntimeError("failed!")

        #
        # When running as root log rotation will not fail as
        # root always has write access.
        #
        if isinstance(platform, Windows) or os.getuid() != 0:
            if (not os.path.isfile("log/client5-4.log") or
                os.stat("log/client5-4.log").st_size < 1024 or
                len(glob.glob("log/client5-4-*.log")) > 0):
                raise RuntimeError("failed!")

            with open("log/client5-4.log", 'r') as f:
                if f.read().count("error: FileLogger: cannot rename `log/client5-4.log'") != 1:
                    raise RuntimeError("failed!")

        print("ok")

    def teardownClientSide(self, current, success):
        self.clean()

    def clean(self):
        for f in glob.glob("client5-*.log"):
            os.remove(f)
        if os.path.exists("log"):
            for f in glob.glob("log/client5-*.log"):
                os.remove(f)
            os.rmdir("log")

TestSuite(__name__, [ IceLoggerTestCase() ], chdir=True)
