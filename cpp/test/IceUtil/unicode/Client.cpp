// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <TestCommon.h>

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

    return EXIT_SUCCESS;
}
