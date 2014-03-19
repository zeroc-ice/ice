// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        PhoneBookPrx phoneBook = PhoneBookPrxHelper.checkedCast(communicator.propertyToProxy("PhoneBook.Proxy"));
        if(phoneBook == null)
        {
            System.err.println(appName + ": invalid object reference");
            return 1;
        }

        Parser parser = new Parser(communicator, phoneBook);
        return parser.parse();
    }
}
