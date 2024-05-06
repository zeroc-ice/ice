//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "OpenSSLUtil.h"

#include <openssl/err.h>

#include <sstream>

//
// Avoid old style cast warnings from OpenSSL macros
//
#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#    // Ignore OpenSSL 3.0 deprecation warning
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;

string
Ice::SSL::OpenSSL::getSslErrors(bool verbose)
{
    ostringstream ostr;

    const char* file;
    const char* data;
    int line;
    int flags;
    unsigned long err;
    int count = 0;
    while ((err = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        if (count > 0)
        {
            ostr << endl;
        }

        if (verbose)
        {
            if (count > 0)
            {
                ostr << endl;
            }

            char buf[200];
            ERR_error_string_n(err, buf, sizeof(buf));

            ostr << "error # = " << err << endl;
            ostr << "message = " << buf << endl;
            ostr << "location = " << file << ", " << line;
            if (flags & ERR_TXT_STRING)
            {
                ostr << endl;
                ostr << "data = " << data;
            }
        }
        else
        {
            const char* reason = ERR_reason_error_string(err);
            ostr << (reason == nullptr ? "unknown reason" : reason);
            if (flags & ERR_TXT_STRING)
            {
                ostr << ": " << data;
            }
        }

        ++count;
    }

    ERR_clear_error();

    return ostr.str();
}
