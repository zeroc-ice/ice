// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslOpenSSLUtils.h>

using std::string;

//
// NOTE: The following (mon, getGeneralizedTime, getUTCTime and getASN1time are routines that
//       have been abducted from the OpenSSL X509 library, and modified to work with the STL
//       basic_string template.

static const char *mon[12]=
{
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

string
IceSecurity::Ssl::OpenSSL::getGeneralizedTime(ASN1_GENERALIZEDTIME *tm)
{
    char buf[30];
    int gmt = 0, y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

    int i = tm->length;

    char* v = (char *) tm->data;

    if (i < 12)
    {
        goto err;
    }

    if (v[i-1] == 'Z')
    {
        gmt=1;
    }

    for (i=0; i<12; i++)
    {
        if ((v[i] > '9') || (v[i] < '0'))
        {
            goto err;
        }
    }

    y = (v[0] - '0') * 1000 + (v[1] - '0') * 100 + (v[2] - '0') * 10 + (v[3] - '0');
    M = (v[4] - '0') * 10 + (v[5] - '0');

    if ((M > 12) || (M < 1))
    {
        goto err;
    }

    d = (v[6] - '0') * 10 + (v[7] - '0');
    h = (v[8] - '0') * 10 + (v[9] - '0');
    m = (v[10] - '0') * 10 + (v[11] - '0');

    if ((v[12] >= '0') && (v[12] <= '9') &&
        (v[13] >= '0') && (v[13] <= '9'))
    {
        s = (v[12] - '0') * 10 + (v[13] - '0');
    }

    sprintf(buf, "%s %2d %02d:%02d:%02d %d%s", mon[M-1], d, h, m, s, y, (gmt)?" GMT":"");
    return string(buf);

err:
    return string("Bad time value");
}

string
IceSecurity::Ssl::OpenSSL::getUTCTime(ASN1_UTCTIME *tm)
{
    char buf[30];
    int gmt = 0, y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

    int i = tm->length;
    char* v = (char *) tm->data;

    if (i < 10)
    { 
        goto err;
    }

    if (v[i-1] == 'Z')
    {
        gmt=1;
    }

    for (i = 0; i < 10; i++)
    {
        if ((v[i] > '9') || (v[i] < '0'))
        {
            goto err;
        }
    }

    y = (v[0] - '0') * 10 + (v[1] - '0');

    if (y < 50)
    {
        y+=100;
    }

    M = (v[2] - '0') * 10 + (v[3] - '0');

    if ((M > 12) || (M < 1))
    {
        goto err;
    }

    d = (v[4] - '0') * 10 + (v[5] - '0');
    h = (v[6] - '0') * 10 + (v[7] - '0');
    m = (v[8] - '0') * 10 + (v[9] - '0');

    if ((v[10] >= '0') && (v[10] <= '9') && (v[11] >= '0') && (v[11] <= '9'))
    {
        s = (v[10] - '0') * 10 + (v[11] - '0');
    }

    sprintf(buf, "%s %2d %02d:%02d:%02d %d%s", mon[M-1], d, h, m, s, y+1900, (gmt)?" GMT":"");
    return string(buf);

err:
    return string("Bad time value");
}

string
IceSecurity::Ssl::OpenSSL::getASN1time(ASN1_TIME *tm)
{
    string theTime;

    switch (tm->type)
    {
        case V_ASN1_UTCTIME :
        {
            theTime = getUTCTime(tm);
        }

        case V_ASN1_GENERALIZEDTIME :
        {
	    theTime = getGeneralizedTime(tm);
        }

        default :
        {
            theTime = "Bad time value";
        }
    }

    return theTime;
}

