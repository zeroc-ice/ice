// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <IceUtil/InputUtil.h>
#include <TestCommon.h>

using namespace IceUtil;
using namespace std;

#define WS " \f\n\r\t\v"

int
main(int, char**)
{
    cout << "testing string-to-64-bit integer conversion... ";

    bool b;
    Int64 result;
    string::size_type pos;

    b = stringToInt64("", result, pos);
    test(!b && result == 0 && pos == string::npos);
    b = stringToInt64(WS, result, pos);
    test(!b && result == 0 && pos == string::npos);

    b = stringToInt64("123", result, pos);
    test(b && result == 123 && pos == string::npos);
    b = stringToInt64("+123", result, pos);
    test(b && result == 123 && pos == string::npos);
    b = stringToInt64("-123", result, pos);
    test(b && result == -123 && pos == string::npos);

    b = stringToInt64("0123", result, pos);
    test(b && result == 83 && pos == string::npos);
    b = stringToInt64("+0123", result, pos);
    test(b && result == 83 && pos == string::npos);
    b = stringToInt64("-0123", result, pos);
    test(b && result == -83 && pos == string::npos);

    b = stringToInt64("0x123", result, pos);
    test(b && result == 291 && pos == string::npos);
    b = stringToInt64("+0x123", result, pos);
    test(b && result == 291 && pos == string::npos);
    b = stringToInt64("-0x123", result, pos);
    test(b && result == -291 && pos == string::npos);

    b = stringToInt64(WS "123", result, pos);
    test(b && result == 123 && pos == string::npos);
    b = stringToInt64("123" WS, result, pos);
    test(b && result == 123 && pos == string::npos);
    b = stringToInt64(WS "123" WS, result, pos);
    test(b && result == 123 && pos == string::npos);

    b = stringToInt64("123Q", result, pos);
    test(b && result == 123 && pos == 3);
    b = stringToInt64(" 123Q", result, pos);
    test(b && result == 123 && pos == 4);
    b = stringToInt64(" 123Q ", result, pos);
    test(b && result == 123 && pos == 4);
    b = stringToInt64(" 123 Q", result, pos);
    test(b && result == 123 && pos == 4);

    b = stringToInt64("Q", result, pos);
    test(!b && result == 0 && pos == 0);
    b = stringToInt64(" Q", result, pos);
    test(!b && result == 0 && pos == 1);

    b = stringToInt64("-9223372036854775807", result, pos);
    test(b && result == ICE_INT64(-9223372036854775807) && pos == string::npos);
    b = stringToInt64("-9223372036854775808", result, pos);
    test(b && result == Int64Min && pos == string::npos);
    b = stringToInt64("-9223372036854775809", result, pos);
    test(!b && result == Int64Min && pos == string::npos);

    b = stringToInt64("9223372036854775806", result, pos);
    test(b && result == ICE_INT64(9223372036854775806) && pos == string::npos);
    b = stringToInt64("9223372036854775807", result, pos);
    test(b && result == Int64Max && pos == string::npos);
    b = stringToInt64("9223372036854775808", result, pos);
    test(!b && result == Int64Max && pos == string::npos);

    b = stringToInt64("-9223372036854775807Q", result, pos);
    test(b && result == ICE_INT64(-9223372036854775807) && pos == 20);
    b = stringToInt64("-9223372036854775808Q", result, pos);
    test(b && result == Int64Min && pos == 20);
    b = stringToInt64("-9223372036854775809Q", result, pos);
    test(!b && result == Int64Min && pos == 20);

    b = stringToInt64("-9223372036854775809999Q", result, pos);
    test(!b && result == Int64Min && pos == 23);

    cout << "ok" << endl;

    return EXIT_SUCCESS;
}
