// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <IceUtil/Time.h>
#include <TestCommon.h>
#include <set>

using namespace IceUtil;
using namespace std;

int
main()
{
    const int howMany = 100000;

    cout << "Generating " << howMany << " UUIds ... " << endl;

    set<string> uuidSet;
    
    Time start = Time::now();
    for (int i = 0; i < howMany; i++)
    {
	pair<set<string>::iterator, bool> ok
	    = uuidSet.insert(generateUUID());
	test(ok.second);
    }
    Time finish = Time::now();
  
    cout << "UUIDs generated every 100 ns: " 
	 << (double) howMany / ((finish - start).toMicroSeconds() * 10) 
	 << endl;
    cout << "ok" << endl;
    
    return EXIT_SUCCESS;
}
