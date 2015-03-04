// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class RunParser
{
    static int
    runParser(String appName, String[] args, Ice.Communicator communicator)
    {
        LibraryPrx library = LibraryPrxHelper.checkedCast(communicator.propertyToProxy("Library.Proxy"));
        if(library == null)
        {
            System.err.println(appName + ": invalid object reference");
            return 1;
        }

        Parser parser = new Parser(communicator, library);
        return parser.parse();
    }
}
