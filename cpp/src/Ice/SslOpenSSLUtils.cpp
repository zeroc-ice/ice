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
#include <Ice/SslSystemInternalF.h>
#include <Ice/SslSystemOpenSSL.h>
#include <Ice/SslFactory.h>
#include <openssl/err.h>
#include <assert.h>

using std::string;

//
// NOTE: The following (mon, getGeneralizedTime, getUTCTime and getASN1time are routines that
//       have been abducted from the OpenSSL X509 library, and modified to work with the STL
//       basic_string template.

//
// TODO: These Diffie-Hellman params have been blatantly stolen from
//       OpenSSL's demo programs.  We SHOULD define our own here, but
//       these will suffice for testing purposes.  Please note, these
//       are not keys themselves, simply a DH Group that allows OpenSSL
//       to create Diffie-Hellman keys.
//

// Instantiation of temporary Diffie-Hellman 512bit key.
unsigned char tempDiffieHellman512p[] =
{
    0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
    0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
    0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
    0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
    0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
    0x47,0x74,0xE8,0x33,
};

unsigned char tempDiffieHellman512g[] =
{
    0x02,
};

static const char *mon[12]=
{
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

string
IceSSL::OpenSSL::getGeneralizedTime(ASN1_GENERALIZEDTIME *tm)
{
    assert(tm != 0);

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
IceSSL::OpenSSL::getUTCTime(ASN1_UTCTIME *tm)
{
    assert(tm != 0);

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
IceSSL::OpenSSL::getASN1time(ASN1_TIME *tm)
{
    assert(tm != 0);

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

DH*
IceSSL::OpenSSL::loadDHParam(const char* dhfile)
{
    assert(dhfile != 0);

    DH* ret = 0;
    BIO* bio;

    if ((bio = BIO_new_file(dhfile,"r")) != 0)
    {
        ret = PEM_read_bio_DHparams(bio, 0, 0, 0);
    }

    if (bio != 0)
    {
        BIO_free(bio);
    }

    return ret;
}

DH*
IceSSL::OpenSSL::getTempDH(unsigned char* p, int plen, unsigned char* g, int glen)
{
    assert(p != 0);
    assert(g != 0);

    DH* dh = 0;

    if ((dh = DH_new()) != 0)
    {
        dh->p = BN_bin2bn(p, plen, 0);

        dh->g = BN_bin2bn(g, glen, 0);

        if ((dh->p == 0) || (dh->g == 0))
        {
            DH_free(dh);
            dh = 0;
        }
    }

    return dh;
}

DH*
IceSSL::OpenSSL::getTempDH512()
{
    DH* dh = getTempDH(tempDiffieHellman512p, sizeof(tempDiffieHellman512p),
                       tempDiffieHellman512g, sizeof(tempDiffieHellman512g));

    return dh;
}

string
IceSSL::OpenSSL::sslGetErrors()
{
    string errorMessage;
    char buf[200];
    char bigBuffer[1024];
    const char* file = 0;
    const char* data = 0;
    int line = 0;
    int flags = 0;
    unsigned errorCode = 0;
    int errorNum = 1;

    unsigned long es = CRYPTO_thread_id();

    while ((errorCode = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        sprintf(bigBuffer,"%6d - Thread ID: %lu\n", errorNum, es);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Error:     %u\n", errorNum, errorCode);
        errorMessage += bigBuffer;

        // Request an error from the OpenSSL library
        ERR_error_string_n(errorCode, buf, sizeof(buf));
        sprintf(bigBuffer,"%6d - Message:   %s\n", errorNum, buf);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Location:  %s, %d\n", errorNum, file, line);
        errorMessage += bigBuffer;

        if (flags & ERR_TXT_STRING)
        {
            sprintf(bigBuffer,"%6d - Data:      %s\n", errorNum, data);
            errorMessage += bigBuffer;
        }

        errorNum++;
    }

    ERR_clear_error();

    return errorMessage;
}

extern "C"
{

RSA*
tmpRSACallback(SSL* sslConnection, int isExport, int keyLength)
{
    assert(sslConnection != 0);

    IceSSL::SystemInternalPtr sslSystem = IceSSL::Factory::getSystemFromHandle(sslConnection);
    assert(sslSystem != 0);

    IceSSL::OpenSSL::System* openSslSystem = dynamic_cast<IceSSL::OpenSSL::System*>(sslSystem.get());
    assert(openSslSystem != 0);

    RSA* rsaKey = openSslSystem->getRSAKey(isExport, keyLength);
    assert(rsaKey != 0);

    return rsaKey;
}

DH*
tmpDHCallback(SSL* sslConnection, int isExport, int keyLength)
{
    assert(sslConnection != 0);

    IceSSL::SystemInternalPtr sslSystem = IceSSL::Factory::getSystemFromHandle(sslConnection);
    assert(sslSystem != 0);

    IceSSL::OpenSSL::System* openSslSystem = dynamic_cast<IceSSL::OpenSSL::System*>(sslSystem.get());
    assert(openSslSystem != 0);

    DH* dh = openSslSystem->getDHParams(isExport, keyLength);
    assert(dh != 0);

    return dh;
}

// verifyCallback - Certificate Verification callback function.
int
verifyCallback(int ok, X509_STORE_CTX* ctx)
{
    assert(ctx != 0);

    // Tricky method to get access to our connection.  I would use SSL_get_ex_data() to get
    // the Connection object, if only I had some way to retrieve the index of the object
    // in this function.  Hence, we have to invent our own reference system here.
    SSL* sslConnection = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    assert(sslConnection != 0);

    IceSSL::OpenSSL::ConnectionPtr connection = IceSSL::OpenSSL::Connection::getConnection(sslConnection);
    assert(connection != 0);

    // Call the connection, get it to perform the verification.
    int retCode = connection->verifyCertificate(ok, ctx);

    return retCode;
}

// TODO: This is a complete hack to get this working again with the CA certificate.
//       Of course, this will have to be rewritten to handle this in the same manner
//       as the verifyCallback does.
//       -ASN
int
passwordCallback(char* buffer, int bufferSize, int rwFlag, void* userData)
{
    strncpy(buffer, "demo", bufferSize);
    buffer[bufferSize - 1] = '\0';
    return strlen(buffer);
}

}

