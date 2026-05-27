// Copyright (c) ZeroC, Inc.

#include "CryptPermissionsVerifier.h"
#include "../Ice/FileUtil.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Ice.h"
#include "Ice/StringUtil.h"

#include <cstdlib>
#include <cstring>
#include <fstream>

#if defined(__GLIBC__)
#    include <crypt.h>
#elif defined(__FreeBSD__)
#    include <unistd.h>
#elif defined(__APPLE__)
#    include <CommonCrypto/CommonCrypto.h>
#    include <CoreFoundation/CoreFoundation.h>
#    include <Security/Security.h>
#elif defined(_WIN32)
#    include <Bcrypt.h>
#    include <Wincrypt.h>
#endif

// Disable deprecation warnings from CommonCrypto APIs
#include "../Ice/DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace Glacier2;

namespace
{
    const char* const cryptPluginName = "Glacier2CryptPermissionsVerifier";

    //
    // Constant-time byte-equality comparison. Returns true iff the two byte ranges of length n are equal.
    // Always reads all n bytes from both inputs to avoid leaking, via execution time, how many leading
    // bytes of the inputs matched. Used to compare hashed/derived-key material in checkPermissions.
    // The `volatile` accumulator prevents the optimizer from turning the loop into an early-exit branch.
    //
    template<typename T> bool constantTimeEquals(const T* a, const T* b, size_t n)
    {
        static_assert(sizeof(T) == 1, "constantTimeEquals operates on byte-sized elements");
        volatile uint8_t diff = 0;
        for (size_t i = 0; i < n; ++i)
        {
            diff = static_cast<uint8_t>(diff | (static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i])));
        }
        return diff == 0;
    }

#if defined(__APPLE__)
    template<typename T> struct CFTypeRefDeleter
    {
        using pointer = T; // This is used by std::unique_ptr to determine the type
        void operator()(T ref) { CFRelease(ref); }
    };
#endif

    class CryptPermissionsVerifierI final : public PermissionsVerifier
    {
    public:
        CryptPermissionsVerifierI(const map<string, string>&);

        bool checkPermissions(string, string, string&, const Ice::Current&) const override;

    private:
        const map<string, string> _passwords;
    };

    class CryptPermissionsVerifierPlugin final : public Ice::Plugin
    {
    public:
        CryptPermissionsVerifierPlugin(CommunicatorPtr);

        void initialize() override;
        void destroy() override;

    private:
        CommunicatorPtr _communicator;
    };

    map<string, string> retrievePasswordMap(const string& file, const LoggerPtr& logger)
    {
        ifstream passwordFile(IceInternal::streamFilename(file).c_str());
        if (!passwordFile)
        {
            string err = IceInternal::lastErrorToString();
            throw Ice::InitializationException(__FILE__, __LINE__, "cannot open '" + file + "' for reading: " + err);
        }
        map<string, string> passwords;

        bool hasDESStylePassword = false;

        while (true)
        {
            string userId;
            passwordFile >> userId;
            if (!passwordFile)
            {
                break;
            }

            string password;
            passwordFile >> password;
            if (!passwordFile)
            {
                break;
            }

            assert(!userId.empty());
            assert(!password.empty());

            hasDESStylePassword = hasDESStylePassword || (password.find('$') == string::npos && password.size() == 13);

            passwords.insert(make_pair(userId, password));
        }

        if (hasDESStylePassword)
        {
            Warning out(logger);
            out << "The password file '" << file << "' contains one or more DES-style passwords. DES is a weak "
                << "algorithm and should not be used in production. ";
        }

        return passwords;
    }

    CryptPermissionsVerifierI::CryptPermissionsVerifierI(const map<string, string>& passwords) : _passwords(passwords)
    {
    }

#if defined(__APPLE__) || defined(_WIN32)

    const char* const padBytes0 = "";
    const char* const padBytes1 = "=";
    const char* const padBytes2 = "==";

    inline string paddingBytes(size_t length)
    {
        switch (length % 4)
        {
            case 2:
            {
                return padBytes2;
            }
            case 3:
            {
                return padBytes1;
            }
            default:
            {
                return padBytes0;
            }
        }
    }
#endif

    bool CryptPermissionsVerifierI::checkPermissions(string userId, string password, string&, const Current&) const
    {
        auto p = _passwords.find(userId);

        if (p == _passwords.end())
        {
            return false;
        }
#if defined(__GLIBC__) || defined(__FreeBSD__)
        size_t i = p->second.rfind('$');
        string salt;
        if (i == string::npos)
        {
            //
            // Crypt DES
            //
            if (p->second.size() != 13) // DES passwords are 13 characters long.
            {
                return false;
            }
            salt = p->second.substr(0, 2);
        }
        else
        {
            salt = p->second.substr(0, i + 1);
            if (salt.empty())
            {
                return false;
            }
        }

        struct crypt_data data;
        data.initialized = 0;
        const char* hashed = crypt_r(password.c_str(), salt.c_str(), &data);

        if (hashed == nullptr || p->second.size() != strlen(hashed))
        {
            return false;
        }
        return constantTimeEquals(p->second.data(), hashed, p->second.size());
#elif defined(__APPLE__) || defined(_WIN32)
        //
        // Pbkdf2 string format:
        //
        // $pbkdf2-digest$rounds$salt$checksum
        // $pbkdf2$rounds$salt$checksum (SHA1 digest)
        //
        size_t beg = 0;
        size_t end = 0;

        //
        // Determine the digest algorithm
        //
#    if defined(__APPLE__)
        CCPseudoRandomAlgorithm algorithmId = 0;
#    else
        LPCWSTR algorithmId = 0;
#    endif
        size_t checksumLength = 0;

        const string pbkdf2SHA1Token = "$pbkdf2$";

        if (p->second.find(pbkdf2SHA1Token) == 0)
        {
#    if defined(__APPLE__)
            algorithmId = kCCPRFHmacAlgSHA1;
#    else
            algorithmId = BCRYPT_SHA1_ALGORITHM;
#    endif
            checksumLength = 20;
            beg = pbkdf2SHA1Token.size();
        }
        else
        {
            //
            // Pbkdf2 string format:
            //
            // $pbkdf2-digest$rounds$salt$checksum
            //
            const string pbkdf2Token = "$pbkdf2-";
            if (p->second.find(pbkdf2Token) != 0)
            {
                return false; // PBKDF2 start token not found
            }

            beg = pbkdf2Token.size();
            end = p->second.find('$', beg);
            if (end == string::npos)
            {
                return false; // Digest algorithm end token not found
            }

            if (p->second.substr(beg, (end - beg)) == "sha256")
            {
#    if defined(__APPLE__)
                algorithmId = kCCPRFHmacAlgSHA256;
#    else
                algorithmId = BCRYPT_SHA256_ALGORITHM;
#    endif
                checksumLength = 32;
            }
            else if (p->second.substr(beg, (end - beg)) == "sha512")
            {
#    if defined(__APPLE__)
                algorithmId = kCCPRFHmacAlgSHA512;
#    else
                algorithmId = BCRYPT_SHA512_ALGORITHM;
#    endif
                checksumLength = 64;
            }
            else
            {
                return false; // Invalid digest algorithm
            }
            beg = end + 1;
        }
        //
        // Determine the number of rounds
        //
        end = p->second.find('$', beg);
        if (end == string::npos)
        {
            return false; // Rounds end token not found
        }

        int64_t rounds;
        try
        {
            rounds = std::stoll(p->second.substr(beg, (end - beg)), nullptr, 0);
        }
        catch (const std::exception&)
        {
            return false; // Invalid rounds value
        }

        //
        // Determine salt and checksum
        //
        beg = end + 1;
        end = p->second.find('$', beg);
        if (end == string::npos)
        {
            return false; // Salt value end token not found
        }

        string salt = p->second.substr(beg, (end - beg));
        string checksum = p->second.substr(end + 1);
        if (checksum.empty())
        {
            return false;
        }

        //
        // passlib encoding is identical to base64 except that it uses . instead of +,
        // and omits trailing padding = and whitepsace.
        //
        std::replace(salt.begin(), salt.end(), '.', '+');
        salt += paddingBytes(salt.size());

        std::replace(checksum.begin(), checksum.end(), '.', '+');
        checksum += paddingBytes(checksum.size());
#    if defined(__APPLE__)
        CFErrorRef error = nullptr;
        unique_ptr<SecTransformRef, CFTypeRefDeleter<SecTransformRef>> decoder(
            SecDecodeTransformCreate(kSecBase64Encoding, &error));

        if (error)
        {
            CFRelease(error);
            return false;
        }

        unique_ptr<CFDataRef, CFTypeRefDeleter<CFDataRef>> data(CFDataCreateWithBytesNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const uint8_t*>(salt.c_str()),
            static_cast<CFIndex>(salt.size()),
            kCFAllocatorNull));

        SecTransformSetAttribute(decoder.get(), kSecTransformInputAttributeName, data.get(), &error);
        if (error)
        {
            CFRelease(error);
            return false;
        }

        unique_ptr<CFDataRef, CFTypeRefDeleter<CFDataRef>> saltBuffer(
            static_cast<CFDataRef>(SecTransformExecute(decoder.get(), &error)));
        if (error)
        {
            CFRelease(error);
            return false;
        }

        vector<uint8_t> checksumBuffer1(checksumLength);
        OSStatus status = CCKeyDerivationPBKDF(
            kCCPBKDF2,
            password.c_str(),
            password.size(),
            CFDataGetBytePtr(saltBuffer.get()),
            static_cast<size_t>(CFDataGetLength(saltBuffer.get())),
            algorithmId,
            static_cast<unsigned int>(rounds),
            checksumBuffer1.data(),
            checksumLength);
        if (status != errSecSuccess)
        {
            return false;
        }

        decoder.reset(SecDecodeTransformCreate(kSecBase64Encoding, &error));
        if (error)
        {
            CFRelease(error);
            return false;
        }

        data.reset(CFDataCreateWithBytesNoCopy(
            kCFAllocatorDefault,
            reinterpret_cast<const uint8_t*>(checksum.c_str()),
            static_cast<CFIndex>(checksum.size()),
            kCFAllocatorNull));

        assert(decoder.get());
        // NOLINTNEXTLINE(clang-analyzer-nullability.NullablePassedToNonnull)
        SecTransformSetAttribute(decoder.get(), kSecTransformInputAttributeName, data.get(), &error);
        if (error)
        {
            CFRelease(error);
            return false;
        }

        data.reset(static_cast<CFDataRef>(SecTransformExecute(decoder.get(), &error)));
        if (error)
        {
            CFRelease(error);
            return false;
        }

        vector<uint8_t> checksumBuffer2(
            CFDataGetBytePtr(data.get()),
            CFDataGetBytePtr(data.get()) + CFDataGetLength(data.get()));
        if (checksumBuffer1.size() != checksumBuffer2.size())
        {
            return false;
        }
        return constantTimeEquals(checksumBuffer1.data(), checksumBuffer2.data(), checksumBuffer1.size());
#    else
        DWORD saltLength = static_cast<DWORD>(salt.size());
        vector<BYTE> saltBuffer(saltLength);

        if (!CryptStringToBinary(
                salt.c_str(),
                static_cast<DWORD>(salt.size()),
                CRYPT_STRING_BASE64,
                &saltBuffer[0],
                &saltLength,
                0,
                0))
        {
            return false;
        }
        saltBuffer.resize(saltLength);

        BCRYPT_ALG_HANDLE algorithmHandle = 0;
        if (BCryptOpenAlgorithmProvider(&algorithmHandle, algorithmId, 0, BCRYPT_ALG_HANDLE_HMAC_FLAG) != 0)
        {
            return false;
        }

        vector<BYTE> checksumBuffer1(checksumLength);

        vector<BYTE> passwordBuffer(password.begin(), password.end());

        DWORD status = BCryptDeriveKeyPBKDF2(
            algorithmHandle,
            &passwordBuffer[0],
            static_cast<DWORD>(passwordBuffer.size()),
            &saltBuffer[0],
            saltLength,
            rounds,
            &checksumBuffer1[0],
            static_cast<DWORD>(checksumLength),
            0);

        BCryptCloseAlgorithmProvider(algorithmHandle, 0);

        if (status != 0)
        {
            return false;
        }

        DWORD checksumBuffer2Length = static_cast<DWORD>(checksumLength);
        vector<BYTE> checksumBuffer2(checksumLength);

        if (!CryptStringToBinary(
                checksum.c_str(),
                static_cast<DWORD>(checksum.size()),
                CRYPT_STRING_BASE64,
                &checksumBuffer2[0],
                &checksumBuffer2Length,
                0,
                0))
        {
            return false;
        }
        if (checksumBuffer2Length != checksumLength)
        {
            return false;
        }
        return constantTimeEquals(checksumBuffer1.data(), checksumBuffer2.data(), checksumLength);
#    endif
#else
#    error "Unsupported platform"
#endif
    }

    CryptPermissionsVerifierPlugin::CryptPermissionsVerifierPlugin(CommunicatorPtr communicator)
        : _communicator(std::move(communicator))
    {
    }

    void CryptPermissionsVerifierPlugin::initialize()
    {
        const string prefix = "Glacier2CryptPermissionsVerifier.";
        const PropertyDict props = _communicator->getProperties()->getPropertiesForPrefix(prefix);

        if (!props.empty())
        {
            ObjectAdapterPtr adapter = _communicator->createObjectAdapter(""); // colloc-only adapter

            // Each prop represents a property to set + the associated password file
            for (const auto& prop : props)
            {
                string name = prop.first.substr(prefix.size());
                Identity id = {Ice::generateUUID(), "Glacier2CryptPermissionsVerifier"};
                auto prx = adapter->add(
                    make_shared<CryptPermissionsVerifierI>(
                        retrievePasswordMap(prop.second, _communicator->getLogger())),
                    id);
                _communicator->getProperties()->setProperty(name, _communicator->proxyToString(prx));
            }

            adapter->activate();
        }
    }

    void CryptPermissionsVerifierPlugin::destroy() {}

    Ice::Plugin*
    createCryptPermissionsVerifier(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
    {
        string pluginName{cryptPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the Glacier2 Crypt Permissions Verifier plug-in must be named '" + pluginName + "'"};
        }

        if (args.size() > 0)
        {
            Error out(communicator->getLogger());
            out << "Plugin " << name << ": too many arguments";
            return nullptr;
        }

        return new CryptPermissionsVerifierPlugin(communicator);
    }
}

Ice::PluginFactory
Glacier2::cryptPermissionsVerifierPluginFactory()
{
    return Ice::PluginFactory{cryptPluginName, createCryptPermissionsVerifier};
}
