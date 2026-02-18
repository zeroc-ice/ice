# Copyright (c) ZeroC, Inc.

import filecmp
import os
import shutil

from IceStormUtil import IceStorm, IceStormTestCase
from Util import Mapping, ProcessFromBinDir, SimpleClient, TestSuite


class IceStormDb(ProcessFromBinDir, SimpleClient):
    def __init__(self, quiet=True, *args, **kargs):
        SimpleClient.__init__(
            self,
            exe="icestormdb",
            quiet=quiet,
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )


icestorm = IceStorm()


class IceStormDbExportImportTestCase(IceStormTestCase):
    def __init__(self):
        IceStormTestCase.__init__(
            self,
            name="icestormdb export/import",
            icestorm=icestorm,
        )

    def runClientSide(self, current):
        testdir = current.testsuite.getPath()

        current.write("creating topics... ")
        self.runadmin(current, "create topic1 topic2 topic3")
        current.writeln("ok")

        # Shut down IceStorm to release the LMDB lock.
        current.write("shutting down IceStorm... ")
        self.shutdown(current)
        self.icestorm[0].stop(current, True)
        current.writeln("ok")

        dbPath = self.icestorm[0].dbdir
        exportFile1 = os.path.join(testdir, "export1.bin")
        exportFile2 = os.path.join(testdir, "export2.bin")
        importDir = os.path.join(testdir, "importdb")

        try:
            current.write("exporting database... ")
            IceStormDb(args=["--export", exportFile1, "--dbpath", dbPath]).run(current)
            current.writeln("ok")

            current.write("importing database... ")
            os.mkdir(importDir)
            IceStormDb(args=["--import", exportFile1, "--dbpath", importDir]).run(current)
            current.writeln("ok")

            current.write("re-exporting imported database... ")
            IceStormDb(args=["--export", exportFile2, "--dbpath", importDir]).run(current)
            current.writeln("ok")

            current.write("comparing export files... ")
            if not filecmp.cmp(exportFile1, exportFile2, shallow=False):
                raise RuntimeError("export files differ: {0} and {1}".format(exportFile1, exportFile2))
            current.writeln("ok")

            # Restart IceStorm from the imported database and verify with icestormadmin.
            current.write("verifying imported database with icestormadmin... ")
            shutil.rmtree(dbPath)
            os.rename(importDir, dbPath)
            importDir = None  # Moved, no longer needs cleanup

            # Bypass setup() to preserve the imported database files.
            self.icestorm[0].setup = lambda c: None
            self.icestorm[0].start(current)

            output = self.runadmin(current, "topics", quiet=True)
            for topic in ["topic1", "topic2", "topic3"]:
                if topic not in output:
                    raise RuntimeError("expected '{0}' in topics, got: {1}".format(topic, output))

            self.shutdown(current)
            self.icestorm[0].stop(current, True)
            current.writeln("ok")
        finally:
            for f in [exportFile1, exportFile2]:
                if os.path.exists(f):
                    os.remove(f)
            if importDir and os.path.exists(importDir):
                shutil.rmtree(importDir)

    def teardownClientSide(self, current, success):
        # IceStorm is already shut down in runClientSide.
        pass


TestSuite(
    __file__,
    [IceStormDbExportImportTestCase()],
    runOnMainThread=True,
    multihost=False,
)
