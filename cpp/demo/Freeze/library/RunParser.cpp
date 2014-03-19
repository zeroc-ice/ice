// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Parser.h>

using namespace std;
using namespace Demo;

int
runParser(int, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    LibraryPrx library = LibraryPrx::checkedCast(communicator->propertyToProxy("Library.Proxy"));
    if(!library)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    ParserPtr p = new Parser(library);
    return p->parse();
}
