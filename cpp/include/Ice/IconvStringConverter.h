// Copyright (c) ZeroC, Inc.

#ifndef ICE_ICONV_STRING_CONVERTER
#define ICE_ICONV_STRING_CONVERTER

//
// For all platforms except Windows
//
#ifndef _WIN32
#    include "Config.h"
#    include "LocalExceptions.h"
#    include "StringConverter.h"
#    include "StringUtil.h"

#    include <algorithm>
#    include <cassert>
#    include <iconv.h>
#    include <langinfo.h>
#    include <memory>
#    include <sstream>

namespace IceInternal
{
    //
    // Converts charT encoded with internalCode to and from UTF-8 byte sequences
    //
    // The implementation allocates a pair of iconv_t on each thread, to avoid
    // opening / closing iconv_t objects all the time.
    //
    //
    template<typename charT> class IconvStringConverter final : public Ice::BasicStringConverter<charT>
    {
    public:
        IconvStringConverter(const std::string&);

        std::byte* toUTF8(const charT*, const charT*, Ice::UTF8Buffer&) const final;

        void fromUTF8(const std::byte*, const std::byte*, std::basic_string<charT>&) const final;

    private:
        struct DescriptorHolder
        {
            std::pair<iconv_t, iconv_t> descriptor;

            // NOLINTNEXTLINE(performance-no-int-to-ptr)
            DescriptorHolder(const std::string& internalCode) : descriptor(iconv_t(-1), iconv_t(-1))
            {
                const char* externalCode = "UTF-8";

                descriptor.first = iconv_open(internalCode.c_str(), externalCode);
                if (descriptor.first == iconv_t(-1)) // NOLINT(performance-no-int-to-ptr)
                {
                    std::ostringstream os;
                    os << "iconv cannot convert from " << externalCode << " to " << internalCode;
                    throw Ice::FeatureNotSupportedException{__FILE__, __LINE__, os.str()};
                }

                descriptor.second = iconv_open(externalCode, internalCode.c_str());
                if (descriptor.second == iconv_t(-1)) // NOLINT(performance-no-int-to-ptr)
                {
                    iconv_close(descriptor.first);
                    std::ostringstream os;
                    os << "iconv cannot convert from " << internalCode << " to " << externalCode;
                    throw Ice::FeatureNotSupportedException{__FILE__, __LINE__, os.str()};
                }
            }

            ~DescriptorHolder()
            {
                [[maybe_unused]] int rs = iconv_close(descriptor.first);
                assert(rs == 0);

                rs = iconv_close(descriptor.second);
                assert(rs == 0);
            }

            DescriptorHolder(const DescriptorHolder&) = delete;
            DescriptorHolder& operator=(const DescriptorHolder&) = delete;
        };

        [[nodiscard]] std::pair<iconv_t, iconv_t> getDescriptors() const;

        const std::string _internalCode;
    };

    //
    // Implementation
    //

    template<typename charT>
    IconvStringConverter<charT>::IconvStringConverter(const std::string& internalCode) : _internalCode(internalCode)
    {
        //
        // Verify that iconv supports conversion to/from internalCode
        //
        const DescriptorHolder descriptorHolder(internalCode);
    }

    template<typename charT> std::pair<iconv_t, iconv_t> IconvStringConverter<charT>::getDescriptors() const
    {
        static const thread_local DescriptorHolder descriptorHolder(_internalCode);
        return descriptorHolder.descriptor;
    }

    template<typename charT>
    std::byte*
    IconvStringConverter<charT>::toUTF8(const charT* sourceStart, const charT* sourceEnd, Ice::UTF8Buffer& buf) const
    {
        iconv_t cd = getDescriptors().second;

        //
        // Reset cd
        //
        [[maybe_unused]] size_t rs = iconv(cd, nullptr, nullptr, nullptr, nullptr);

        char* inbuf = reinterpret_cast<char*>(const_cast<charT*>(sourceStart));
        size_t inbytesleft = static_cast<size_t>(sourceEnd - sourceStart) * sizeof(charT);
        std::byte* outbuf = nullptr;

        size_t count = 0;
        //
        // Loop while we need more buffer space
        //
        do
        {
            size_t howMany = std::max(inbytesleft, size_t(4));
            outbuf = buf.getMoreBytes(howMany, outbuf);
            count = iconv(cd, &inbuf, &inbytesleft, reinterpret_cast<char**>(&outbuf), &howMany);
        } while (count == size_t(-1) && errno == E2BIG);

        if (count == size_t(-1))
        {
            int errorCode = errno;
            std::ostringstream os;
            os << "iconv failed with: " << (errorCode == 0 ? "unknown error" : IceInternal::errorToString(errorCode));
            throw Ice::IllegalConversionException(__FILE__, __LINE__, os.str());
        }
        return outbuf;
    }

    template<typename charT>
    void IconvStringConverter<charT>::fromUTF8(
        const std::byte* sourceStart,
        const std::byte* sourceEnd,
        std::basic_string<charT>& target) const
    {
        iconv_t cd = getDescriptors().first;

        //
        // Reset cd
        //
        [[maybe_unused]] size_t rs = iconv(cd, nullptr, nullptr, nullptr, nullptr);
        assert(rs == 0);
        char* inbuf = reinterpret_cast<char*>(const_cast<std::byte*>(sourceStart));
        assert(sourceEnd > sourceStart);
        auto inbytesleft = static_cast<size_t>(sourceEnd - sourceStart);

        char* outbuf = nullptr;
        size_t outbytesleft = 0;
        size_t count = 0;

        //
        // Loop while we need more buffer space
        //
        do
        {
            size_t bytesUsed = 0;
            if (outbuf)
            {
                bytesUsed = static_cast<size_t>(outbuf - reinterpret_cast<const char*>(target.data()));
            }

            const size_t increment = std::max<size_t>(inbytesleft, 4);
            target.resize(target.size() + increment);
            outbuf = const_cast<char*>(reinterpret_cast<const char*>(target.data())) + bytesUsed;
            outbytesleft += increment * sizeof(charT);

            count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

        } while (count == size_t(-1) && errno == E2BIG);

        if (count == size_t(-1))
        {
            int errorCode = errno;
            std::ostringstream os;
            os << "iconv failed with: " << (errorCode == 0 ? "unknown error" : IceInternal::errorToString(errorCode));
            throw Ice::IllegalConversionException(__FILE__, __LINE__, os.str());
        }

        target.resize(target.size() - (outbytesleft / sizeof(charT)));
    }
}

namespace Ice
{
    /// Creates a string converter for the given code.
    /// @param internalCodeWithDefault The desired code. If empty or not provided, a default code is used.
    /// @return The converter object.
    /// @throws FeatureNotSupportedException If the code is not supported.
    template<typename charT>
    std::shared_ptr<Ice::BasicStringConverter<charT>>
    createIconvStringConverter(const std::string& internalCodeWithDefault = "")
    {
        std::string internalCode = internalCodeWithDefault;

        if (internalCode.empty())
        {
            internalCode = nl_langinfo(CODESET);
        }

        return std::make_shared<IceInternal::IconvStringConverter<charT>>(internalCode);
    }
}

#endif
#endif
