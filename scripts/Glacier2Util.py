#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, os
from Util import *

class Glacier2Router(ProcessIsFromPath, ProcessIsReleaseOnly, Server):

    def __init__(self, portnum=50, passwords={"userid": "abc123"}, *args, **kargs):
        Server.__init__(self, "glacier2router", mapping=Mapping.getByName("cpp"), desc="Glacier2 router",
                        readyCount=2, *args, **kargs)
        self.portnum = portnum
        self.passwords = passwords

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current):
        if self.passwords:
            path = os.path.join(current.testsuite.getPath(), "passwords")
            with open(path, "w") as file:
                command = "\"%s\" %s" % (sys.executable,
                                     os.path.abspath(os.path.join(toplevel, "scripts", "icehashpassword.py")))

                #
                # For Linux ARM default rounds makes test slower (Usually runs on embedded boards)
                #
                if current.config.buildPlatform.find("arm") >= 0:
                    command += " --rounds 100000"

                for user, password in self.passwords.items():
                    file.write("%s %s\n" % (user, run(command, stdin=(password + '\r\n').encode('UTF-8'))))
            current.files.append(path)

    def getProps(self, current):
        props = Server.getProps(self, current)

        props.update({
            "Glacier2.Client.Endpoints" : current.getTestEndpoint(self.portnum),
            "Glacier2.Server.Endpoints" : current.config.transport,
            "Ice.Admin.Endpoints" : current.getTestEndpoint(self.portnum + 1),
            "Ice.Admin.InstanceName" : "Glacier2",
        })
        if self.passwords:
            props["Glacier2.CryptPasswords"] = os.path.join(current.testsuite.getPath(), "passwords")
        if isinstance(current.testcase.getTestSuite(), Glacier2TestSuite):
            # Add the properties provided by the Glacier2TestSuite routerProps parameter.
            props.update(current.testcase.getTestSuite().getRouterProps(self, current))
        return props

    def getClientProxy(self, current):
        return "Glacier2/router:{0}".format(current.getTestEndpoint(self.portnum))

class Glacier2TestSuite(TestSuite):

    def __init__(self, path, routerProps={}, testcases=None, *args, **kargs):
        if testcases is None:
            testcases = [ClientServerTestCase(servers=[Glacier2Router(), Server()])]
        TestSuite.__init__(self, path, testcases, *args, **kargs)
        self.routerProps = routerProps

    def getRouterProps(self, process, current):
        return self.routerProps(process, current) if callable(self.routerProps) else self.routerProps.copy()
