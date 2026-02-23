# Copyright (c) ZeroC, Inc.

import filecmp
import os
import shutil

from IceGridUtil import IceGridRegistryMaster, IceGridTestCase
from Util import Mapping, ProcessFromBinDir, SimpleClient, TestSuite, Windows, platform


class IceGridDb(ProcessFromBinDir, SimpleClient):
    def __init__(self, quiet=True, *args, **kargs):
        SimpleClient.__init__(
            self,
            exe="icegriddb",
            quiet=quiet,
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )


class IceGridDbExportImportTestCase(IceGridTestCase):
    def __init__(self):
        IceGridTestCase.__init__(
            self,
            name="icegriddb export/import",
            icegridregistry=[IceGridRegistryMaster()],
        )

    def runClientSide(self, current):
        testdir = current.testsuite.getPath()

        # Shut down the node and registry to release the LMDB lock.
        current.write("shutting down IceGrid... ")
        self.runadmin(current, "node shutdown localnode")
        self.icegridnode[0].stop(current, True)
        self.runadmin(current, "registry shutdown Master")
        self.icegridregistry[0].stop(current, True)
        current.writeln("ok")

        dbPath = self.icegridregistry[0].dbdir
        exportFile1 = os.path.join(testdir, "export1.bin")
        exportFile2 = os.path.join(testdir, "export2.bin")
        importDir = os.path.join(testdir, "importdb")

        try:
            current.write("exporting database... ")
            IceGridDb(args=["--export", exportFile1, "--dbpath", dbPath]).run(current)
            current.writeln("ok")

            current.write("importing database... ")
            os.mkdir(importDir)
            IceGridDb(args=["--import", exportFile1, "--dbpath", importDir]).run(current)
            current.writeln("ok")

            current.write("re-exporting imported database... ")
            IceGridDb(args=["--export", exportFile2, "--dbpath", importDir]).run(current)
            current.writeln("ok")

            current.write("comparing export files... ")
            if not filecmp.cmp(exportFile1, exportFile2, shallow=False):
                raise RuntimeError("export files differ: {0} and {1}".format(exportFile1, exportFile2))
            current.writeln("ok")

            # Restart the registry from the imported database and verify with icegridadmin.
            current.write("verifying imported database with icegridadmin... ")
            shutil.rmtree(dbPath)
            os.rename(importDir, dbPath)
            importDir = None  # Moved, no longer needs cleanup

            # Bypass setup() to preserve the imported database files.
            self.icegridregistry[0].setup = lambda c: None
            self.icegridregistry[0].start(current)

            output = self.runadmin(current, "application list", quiet=True)
            if "Test" not in output:
                raise RuntimeError("expected 'Test' in application list, got: " + output)

            self.runadmin(current, "registry shutdown Master")
            self.icegridregistry[0].stop(current, True)
            current.writeln("ok")
        finally:
            for f in [exportFile1, exportFile2]:
                if os.path.exists(f):
                    os.remove(f)
            if importDir and os.path.exists(importDir):
                shutil.rmtree(importDir)

    def teardownClientSide(self, current, success):
        # Registry and node are already shut down in runClientSide.
        pass


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [IceGridDbExportImportTestCase()],
        runOnMainThread=True,
        multihost=False,
    )
