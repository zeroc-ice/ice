# Copyright (c) ZeroC, Inc.

import os
import re

from Util import ClientTestCase, SliceTranslator, TestSuite


class SliceUtf8BomTestCase(ClientTestCase):
    def runClientSide(self, current):
        # Make sure the BOM marker wasn't accidentally deleted by over-zealous reformatting.
        with open("Test.ice", 'rb') as test_file:
            BOM = test_file.read(3)
            if (BOM[0] != 0xEF or BOM[1] != 0xBB or BOM[2] != 0xBF):
                raise RuntimeException("BOM marker is missing from 'Slice/utf8BOM/Test.ice'!")


TestSuite(__name__, [SliceUtf8BomTestCase()], chdir=True)
