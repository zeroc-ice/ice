// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// UWP Certificate implementation
//

#include <IceSSL/UWP.h>
#include <IceSSL/CertificateI.h>
#include <IceSSL/Util.h>

#include <Ice/LocalException.h>
#include <Ice/StringConverter.h>
#include <Ice/Base64.h>
#include <Ice/StringUtil.h>

#include <ppltasks.h>
#include <nserror.h>

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;

using namespace Ice;
using namespace IceSSL;

using namespace std;

namespace
{

class UWPCertificateI : public UWP::Certificate,
                        public CertificateI
{
public:

    UWPCertificateI(Windows::Security::Cryptography::Certificates::Certificate^);

    virtual bool operator==(const IceSSL::Certificate&) const;

    virtual vector<Ice::Byte> getAuthorityKeyIdentifier() const;
    virtual vector<Ice::Byte> getSubjectKeyIdentifier() const;
    virtual bool verify(const CertificatePtr&) const;
    virtual string encode() const;

    virtual chrono::system_clock::time_point getNotAfter() const;
    virtual chrono::system_clock::time_point getNotBefore() const;
    virtual string getSerialNumber() const;
    virtual DistinguishedName getIssuerDN() const;
    virtual vector<pair<int, string> > getIssuerAlternativeNames() const;
    virtual DistinguishedName getSubjectDN() const;
    virtual vector<pair<int, string> > getSubjectAlternativeNames() const;
    virtual int getVersion() const;
    virtual Windows::Security::Cryptography::Certificates::Certificate^ getCert() const;

private:

    Windows::Security::Cryptography::Certificates::Certificate^ _cert;
};

const Ice::Long TICKS_PER_MSECOND = 10000LL;
const Ice::Long MSECS_TO_EPOCH = 11644473600000LL;

vector<pair<int, string> >
certificateAltNames(Windows::Security::Cryptography::Certificates::SubjectAlternativeNameInfo^ subAltNames)
{
    vector<pair<int, string> > altNames;
    if(subAltNames)
    {
        for(auto iter = subAltNames->EmailName->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameEmail, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->DnsName->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameDNS, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->Url->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNameURL, wstringToString(iter->Current->Data())));
        }
        for(auto iter = subAltNames->IPAddress->First(); iter->HasCurrent; iter->MoveNext())
        {
            altNames.push_back(make_pair(AltNAmeIP, wstringToString(iter->Current->Data())));
        }
    }
    return altNames;
}

} // end anonymous namespace

UWPCertificateI::UWPCertificateI(Windows::Security::Cryptography::Certificates::Certificate^ cert) : _cert(cert)
{
    if(!_cert)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Invalid certificate reference");
    }
}

bool
UWPCertificateI::operator==(const IceSSL::Certificate& r) const
{
    const UWPCertificateI* p = dynamic_cast<const UWPCertificateI*>(&r);
    if(!p)
    {
        return false;
    }

    return CryptographicBuffer::Compare(_cert->GetCertificateBlob(), p->_cert->GetCertificateBlob());
}

vector<Ice::Byte>
UWPCertificateI::getAuthorityKeyIdentifier() const
{
    throw  FeatureNotSupportedException(__FILE__, __LINE__);
}

vector<Ice::Byte>
UWPCertificateI::getSubjectKeyIdentifier() const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

bool
UWPCertificateI::verify(const CertificatePtr&) const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

string
UWPCertificateI::encode() const
{
    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(_cert->GetCertificateBlob());
    std::vector<unsigned char> data(reader->UnconsumedBufferLength);
    if(!data.empty())
    {
        reader->ReadBytes(Platform::ArrayReference<unsigned char>(&data[0], static_cast<unsigned int>(data.size())));
    }
    ostringstream os;
    os << "-----BEGIN CERTIFICATE-----\n";
    os << IceInternal::Base64::encode(data);
    os << "-----END CERTIFICATE-----\n";
    return os.str();
}

chrono::system_clock::time_point
UWPCertificateI::getNotAfter() const
{
    // Convert 100ns time from January 1, 1601 to ms from January 1, 1970
    return chrono::system_clock::time_point(
        chrono::milliseconds(_cert->ValidTo.UniversalTime / TICKS_PER_MSECOND - MSECS_TO_EPOCH));
}

chrono::system_clock::time_point
UWPCertificateI::getNotBefore() const
{
    // Convert 100ns time from January 1, 1601 to ms from January 1, 1970
    return chrono::system_clock::time_point(
        chrono::milliseconds(_cert->ValidFrom.UniversalTime / TICKS_PER_MSECOND - MSECS_TO_EPOCH));
}

string
UWPCertificateI::getSerialNumber() const
{
    ostringstream os;
    os.fill(0);
    os.width(2);
    for(unsigned int i = 0; i < _cert->SerialNumber->Length; i++)
    {
        os << hex << static_cast<int>(_cert->SerialNumber[i]);
    }
    return IceUtilInternal::toUpper(os.str());
}

DistinguishedName
UWPCertificateI::getIssuerDN() const
{
    return DistinguishedName("CN=" + wstringToString(_cert->Issuer->Data()));
}

vector<pair<int, string> >
UWPCertificateI::getIssuerAlternativeNames() const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

DistinguishedName
UWPCertificateI::getSubjectDN() const
{
    return DistinguishedName("CN=" + wstringToString(_cert->Subject->Data()));
}

vector<pair<int, string> >
UWPCertificateI::getSubjectAlternativeNames() const
{
    return certificateAltNames(_cert->SubjectAlternativeName);
}

int
UWPCertificateI::getVersion() const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

Windows::Security::Cryptography::Certificates::Certificate^
UWPCertificateI::getCert() const
{
    return _cert;
}

UWP::CertificatePtr
UWP::Certificate::create(Windows::Security::Cryptography::Certificates::Certificate^ cert)
{
    return ICE_MAKE_SHARED(UWPCertificateI, cert);
}

UWP::CertificatePtr
UWP::Certificate::load(const std::string& file)
{
    try
    {
        auto uri = ref new Uri(ref new String(stringToWstring(file).c_str()));
        auto file = create_task(StorageFile::GetFileFromApplicationUriAsync(uri)).get();
        auto buffer = create_task(FileIO::ReadTextAsync(file)).get();
        return UWP::Certificate::decode(wstringToString(buffer->Data()));
    }
    catch(Platform::Exception^ ex)
    {
        if(HRESULT_CODE(ex->HResult) == ERROR_FILE_NOT_FOUND)
        {
            throw CertificateReadException(__FILE__, __LINE__, "error opening file :" + file);
        }
        else
        {
            throw Ice::SyscallException(__FILE__, __LINE__, ex->HResult);
        }
    }
}

UWP::CertificatePtr
UWP::Certificate::decode(const std::string& encoding)
{
    string::size_type size, startpos, endpos = 0;
    startpos = encoding.find("-----BEGIN CERTIFICATE-----", endpos);
    if (startpos != string::npos)
    {
        startpos += sizeof("-----BEGIN CERTIFICATE-----");
        endpos = encoding.find("-----END CERTIFICATE-----", startpos);
        size = endpos - startpos;
    }
    else
    {
        startpos = 0;
        endpos = string::npos;
        size = encoding.size();
    }

    vector<unsigned char> data(IceInternal::Base64::decode(string(&encoding[startpos], size)));
    auto writer = ref new DataWriter();
    writer->WriteBytes(Platform::ArrayReference<unsigned char>(&data[0], static_cast<unsigned int>(data.size())));
    return make_shared<UWPCertificateI>(ref new Certificates::Certificate(writer->DetachBuffer()));
}
