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

#if defined(_MSC_VER)
const Int64 Int64Min = -9223372036854775808i64;
const Int64 Int64Max =  9223372036854775807i64;
#elif defined(ICE_64)
const Int64 Int64Min = -0x7fffffffffffffffL-1L;
const Int64 Int64Max = 0x7fffffffffffffffL;
#else
const Int64 Int64Min = -0x7fffffffffffffffLL-1LL;
const Int64 Int64Max = 0x7fffffffffffffffLL;
#endif

int
main(int, char**)
{
    cout << "testing string-to-64-bit integer conversion... ";

    bool b;
    Int64 result;

    b = stringToInt64("", result);
    test(!b && result == 0);
    b = stringToInt64(WS, result);
    test(!b && result == 0);

    b = stringToInt64("123", result);
    test(b && result == 123);
    b = stringToInt64("+123", result);
    test(b && result == 123);
    b = stringToInt64("-123", result);
    test(b && result == -123);

    b = stringToInt64("0123", result);
    test(b && result == 83);
    b = stringToInt64("+0123", result);
    test(b && result == 83);
    b = stringToInt64("-0123", result);
    test(b && result == -83);

    b = stringToInt64("0x123", result);
    test(b && result == 291);
    b = stringToInt64("+0x123", result);
    test(b && result == 291);
    b = stringToInt64("-0x123", result);
    test(b && result == -291);

    b = stringToInt64(WS "123", result);
    test(b && result == 123);
    b = stringToInt64("123" WS, result);
    test(b && result == 123);
    b = stringToInt64(WS "123" WS, result);
    test(b && result == 123);

    b = stringToInt64("123Q", result);
    test(b && result == 123);
    b = stringToInt64(" 123Q", result);
    test(b && result == 123);
    b = stringToInt64(" 123Q ", result);
    test(b && result == 123);
    b = stringToInt64(" 123 Q", result);
    test(b && result == 123);

    b = stringToInt64("Q", result);
    test(!b && result == 0);
    b = stringToInt64(" Q", result);
    test(!b && result == 0);

    b = stringToInt64("-9223372036854775807", result);
    test(b && result == -ICE_INT64(9223372036854775807));
    b = stringToInt64("-9223372036854775808", result);
    test(b && result == Int64Min);
    b = stringToInt64("-9223372036854775809", result);
    test(!b && result < 0);

    b = stringToInt64("9223372036854775806", result);
    test(b && result == ICE_INT64(9223372036854775806));
    b = stringToInt64("9223372036854775807", result);
    test(b && result == Int64Max);
    b = stringToInt64("9223372036854775808", result);
    test(!b && result > 0);

    b = stringToInt64("-9223372036854775807Q", result);
    test(b && result == -ICE_INT64(9223372036854775807));
    b = stringToInt64("-9223372036854775808Q", result);
    test(b && result == Int64Min);
    b = stringToInt64("-9223372036854775809Q", result);
    test(!b && result < 0);

    b = stringToInt64("-9223372036854775809999Q", result);
    test(!b && result < 0);

    cout << "ok" << endl;

    return EXIT_SUCCESS;
}
