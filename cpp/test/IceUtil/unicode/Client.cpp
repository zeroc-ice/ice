// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <TestCommon.h>
#include <fstream>

using namespace IceUtil;
using namespace std;

int
main(int, char**)
{
    cout << "testing string/wstring conversion... ";

    string arabic = "لماذا لا يتكلمون اللّغة العربية فحسب؟";

    wstring warabic = stringToWstring(arabic);
    test(warabic.length() == 37);
    
    string arabic2 = wstringToString(warabic);
    test(arabic2.length() == arabic.length());
    test(arabic2 == arabic);

    string japanese = "なぜ、みんな日本語を話してくれないのか？";

    wstring wjapanese = stringToWstring(japanese);
    test(wjapanese.length() == 20);
    
    string japanese2 = wstringToString(wjapanese);
    test(japanese2.length() == japanese.length());
    test(japanese2 == japanese);
 
    cout << "ok" << endl;

    cout << "ditto, but with random unicode text... ";

    ifstream numeric("numeric.txt");
    test(numeric.good());
    wstring wrandom;
    while(numeric)
    {
	int c;
	numeric >> c;
	if(numeric)
	{
	    wrandom += static_cast<wchar_t>(c);
	}
    }
    numeric.close();

    ifstream utf8("utf8.txt");
    test(utf8.good());
    string random;
    while(utf8)
    {
	char c;
	utf8.get(c);
	if(utf8)
	{
	    random += c;
	}
    }
    utf8.close();

    string random2 = wstringToString(wrandom);
    wstring wrandom2 = stringToWstring(random);

/*
    unsigned int i;

    ofstream numeric2("numeric2.txt");
    for(i = 0; i < wrandom2.length(); ++i)
    {
	numeric2 << static_cast<int>(wrandom2[i]) << '\n';
    }
    numeric2.close();

    ofstream utf82("utf82.txt");
    for(i = 0; i < random2.length(); ++i)
    {
	utf82.put(random2[i]);
    }
    utf82.close();
*/

    test (random2.length() == random.length());
    test (wrandom2.length() == wrandom.length());

    test (random2 == random);
    test (wrandom2 == wrandom);

    cout << "ok" << endl;

    return EXIT_SUCCESS;
}
