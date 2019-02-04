//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/Config.h>

#include <IceSSL/UWPEngineF.h>
#include <IceSSL/UWPEngine.h>
#include <IceSSL/UWPTransceiverI.h>
#include <IceSSL/Util.h>

#include <Ice/Communicator.h>
#include <Ice/StringConverter.h>
#include <Ice/StringUtil.h>
#include <Ice/Properties.h>
#include <Ice/Network.h>
#include <IceUtil/Shared.h>

#include <string>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceSSL;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Security::Cryptography::Certificates;

IceUtil::Shared* UWP::upCast(UWP::SSLEngine* p) { return p; }

namespace
{

//
// Find a certificate in the Application Personal certificate store
// with the given friendly name. Returns the matching certificate or
// nullptr if none is found.
//
Certificates::Certificate^
findPersonalCertificate(String^ friendlyName)
{
    CertificateQuery^ query = ref new CertificateQuery();
    query->IncludeDuplicates = true;
    query->IncludeExpiredCertificates = true;
    query->FriendlyName = friendlyName;
    query->StoreName = StandardCertificateStoreNames::Personal;

    try
    {
        auto certificates = IceInternal::runSync(CertificateStores::FindAllAsync(query));
        return certificates->Size > 0 ? certificates->GetAt(0) : nullptr;
    }
    catch(Platform::Exception^ ex)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: certificate error:\n" + wstringToString(ex->Message->Data()));
    }
}

//
// Import a certificate in the Application Personal certificate store
// with the given friendly name. Returns true if there was a password
// error and false otherwise. If the import fails because a different
// error PluginInitializationException exception is throw.
//
bool
importPfxData(String^ friendlyName, String^ data, String^ password)
{
    try
    {
        IceInternal::runSync(CertificateEnrollmentManager::ImportPfxDataAsync(
            data,
            password,
            ExportOption::NotExportable,
            KeyProtectionLevel::NoConsent,
            InstallOptions::None,
            friendlyName));
        return false; // The import succcess
    }
    catch(Platform::Exception^ ex)
    {
        if(HRESULT_CODE(ex->HResult) == ERROR_DECRYPTION_FAILED)
        {
            return true; // Password error
        }
        else
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: certificate error:\n" + wstringToString(ex->Message->Data()));
        }
    }
}

Certificates::Certificate^
importPersonalCertificate(const string& filename, function<string ()> password, bool passwordPrompt,
                          int passwordRetryMax)
{
    auto uri = ref new Uri(ref new String(stringToWstring(filename).c_str()));
    try
    {
        auto file = IceInternal::runSync(StorageFile::GetFileFromApplicationUriAsync(uri));
        auto buffer = IceInternal::runSync(FileIO::ReadBufferAsync(file));

        //
        // Create a hash of the certificate to use as a friendly name, this will allow us
        // to uniquely identify the certificate in the store.
        //
        auto hasher = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha1);
        auto hash = hasher->CreateHash();

        hash->Append(buffer);
        String^ friendlyName = CryptographicBuffer::EncodeToBase64String(hash->GetValueAndReset());

        //
        // If the certificate is already in the store we avoid importing it.
        //
        Certificates::Certificate^ cert = findPersonalCertificate(friendlyName);
        if(cert)
        {
            return cert;
        }
        else
        {
            String^ data = CryptographicBuffer::EncodeToBase64String(buffer);
            int count = 0;
            bool passwordErr = false;
            do
            {
                passwordErr = importPfxData(friendlyName, data,
                                            ref new String(stringToWstring(password()).c_str()));
            }
            while(passwordPrompt && passwordErr && ++count < passwordRetryMax);
            if(passwordErr)
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: error decoding certificate");
            }
            return findPersonalCertificate(friendlyName);
        }
    }
    catch(Platform::Exception^ ex)
    {
        if(HRESULT_CODE(ex->HResult) == ERROR_FILE_NOT_FOUND)
        {
            throw PluginInitializationException(__FILE__, __LINE__, "certificate file not found:\n" + filename);
        }
        else
        {
            throw PluginInitializationException(__FILE__, __LINE__,
                                                "IceSSL: certificate error:\n" + wstringToString(ex->Message->Data()));
        }
    }
}

IVectorView<Certificates::Certificate^>^
findCertificates(const string& name, const string& value)
{
    CertificateQuery^ query = ref new CertificateQuery();
    query->StoreName = ref new String(stringToWstring(name).c_str());
    query->IncludeDuplicates = true;
    query->IncludeExpiredCertificates = true;

    if(value != "*")
    {
        if(value.find(':', 0) == string::npos)
        {
            throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: no key in `" + value + "'");
        }
        size_t start = 0;
        size_t pos;
        while((pos = value.find(':', start)) != string::npos)
        {
            string field = IceUtilInternal::toUpper(IceUtilInternal::trim(value.substr(start, pos - start)));
            if(field != "ISSUER" && field != "THUMBPRINT" && field != "FRIENDLYNAME")
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unknown key in `" + value + "'");
            }

            start = pos + 1;
            while(start < value.size() && (value[start] == ' ' || value[start] == '\t'))
            {
                ++start;
            }

            if(start == value.size())
            {
                throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: missing argument in `" + value + "'");
            }

            string arg;
            if(value[start] == '"' || value[start] == '\'')
            {
                size_t end = start;
                ++end;
                while(end < value.size())
                {
                    if(value[end] == value[start] && value[end - 1] != '\\')
                    {
                        break;
                    }
                    ++end;
                }
                if(end == value.size() || value[end] != value[start])
                {
                    throw PluginInitializationException(__FILE__, __LINE__, "IceSSL: unmatched quote in `" + value + "'");
                }
                ++start;
                arg = value.substr(start, end - start);
                start = end + 1;
            }
            else
            {
                size_t end = value.find_first_of(" \t", start);
                if(end == string::npos)
                {
                    arg = value.substr(start);
                    start = value.size();
                }
                else
                {
                    arg = value.substr(start, end - start);
                    start = end + 1;
                }
            }

            if(field == "ISSUER")
            {
                query->IssuerName = ref new String(stringToWstring(arg).c_str());
            }
            else if(field == "FRIENDLYNAME")
            {
                query->FriendlyName = ref new String(stringToWstring(arg).c_str());
            }
            else if(field == "THUMBPRINT")
            {
                vector<BYTE> buffer;
                if(!parseBytes(arg, buffer))
                {
                    throw PluginInitializationException(__FILE__, __LINE__,
                        "IceSSL: invalid `IceSSL.FindCert' property: can't decode the value");
                }
                query->Thumbprint = ref new Array<unsigned char>(&buffer[0], static_cast<unsigned int>(buffer.size()));
            }
        }
    }

    try
    {
        return IceInternal::runSync(CertificateStores::FindAllAsync(query));
    }
    catch(Platform::Exception^ ex)
    {
        throw PluginInitializationException(__FILE__, __LINE__,
                                            "IceSSL: certificate error:\n" + wstringToString(ex->Message->Data()));
    }
}

} // anonymous namespace end

UWP::SSLEngine::SSLEngine(const Ice::CommunicatorPtr& communicator) : IceSSL::SSLEngine(communicator)
{
}

void
UWP::SSLEngine::initialize()
{
    Mutex::Lock lock(_mutex);
    if(_initialized)
    {
        return;
    }

    IceSSL::SSLEngine::initialize();

    const auto properties = communicator()->getProperties();

    //
    // Load client certificate
    //
    const int passwordRetryMax = properties->getPropertyAsIntWithDefault("IceSSL.PasswordRetryMax", 3);
    setPassword(properties->getProperty("IceSSL.Password"));

    string certFile = properties->getProperty("IceSSL.CertFile");
    string findCert = properties->getProperty("IceSSL.FindCert");
    if(!certFile.empty())
    {
        _certificate = dynamic_pointer_cast<UWP::Certificate>(UWP::Certificate::create(importPersonalCertificate(
            certFile,
            [this]()
            {
                return password(false);
            },
            getPasswordPrompt() != nullptr,
            passwordRetryMax)));
    }
    else if(!findCert.empty())
    {
        auto certs = findCertificates(properties->getPropertyWithDefault("IceSSL.CertStore", "My"), findCert);
        if(certs->Size == 0)
        {
            throw Ice::PluginInitializationException(__FILE__, __LINE__, "IceSSL: no certificates found");
        }
        _certificate = dynamic_pointer_cast<UWP::Certificate>(UWP::Certificate::create(certs->GetAt(0)));
    }
    _initialized = true;
}

bool
UWP::SSLEngine::initialized() const
{
    return _initialized;
}

shared_ptr<UWP::Certificate>
UWP::SSLEngine::certificate()
{
    return _certificate;
}

void
UWP::SSLEngine::destroy()
{
}

IceInternal::TransceiverPtr
UWP::SSLEngine::createTransceiver(const InstancePtr& instance,
                                  const IceInternal::TransceiverPtr& delegate,
                                  const string& hostOrAdapterName,
                                  bool incoming)
{
    return new UWP::TransceiverI(instance, delegate, hostOrAdapterName, incoming);
}
