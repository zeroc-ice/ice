// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <StringConverterI.h>

using namespace std;

Demo::StringConverterI::StringConverterI()
{
}

Demo::StringConverterI::~StringConverterI()
{
}

Ice::Byte*
Demo::StringConverterI::toUTF8(const char* sourceStart, const char* sourceEnd, Ice::UTF8Buffer& buffer) const
{
    size_t inputSize = static_cast<size_t>(sourceEnd - sourceStart);
    size_t chunkSize = std::max<size_t>(inputSize, 6);
    size_t outputBytesLeft = chunkSize;
    
    Ice::Byte* targetStart = buffer.getMoreBytes(chunkSize, 0);
    size_t offset = 0;

    for(unsigned int i = 0; i < inputSize; ++i)
    {
        unsigned char byte = sourceStart[i];
        if(byte <= 0x7F)
        {
            if(outputBytesLeft == 0)
            {
                targetStart = buffer.getMoreBytes(chunkSize, targetStart + chunkSize);
                offset = 0;
            }

            targetStart[offset] = byte;

            ++offset;
            --outputBytesLeft;
        }
        else
        {
            if(outputBytesLeft <= 1)
            {
                targetStart = buffer.getMoreBytes(chunkSize, targetStart + chunkSize - outputBytesLeft);
                offset = 0;
            }

            targetStart[offset] = 0xC0 | ((byte & 0xC0) >> 6); 
            targetStart[offset + 1] = 0x80 | (byte & 0x3F);

            offset += 2;
            outputBytesLeft -= 2;
        }
    }

    return targetStart + offset;
}

void
Demo::StringConverterI::fromUTF8(const Ice::Byte* sourceStart, const Ice::Byte* sourceEnd, 
                                 string& target) const
{
    size_t inSize = static_cast<size_t>(sourceEnd - sourceStart);
    target.resize(inSize);

    unsigned int targetIndex = 0;
    unsigned int i = 0;
    while(i < inSize)
    {
        if((sourceStart[i] & 0xC0) == 0xC0)
        {
            if(i + 1 >= inSize)
            {
                throw Ice::StringConversionException(__FILE__, __LINE__, "UTF-8 string source exhausted");
            }
            target[targetIndex] = (sourceStart[i] & 0x03) << 6;
            target[targetIndex] = target[targetIndex] | (sourceStart[i + 1] & 0x3F);
            i += 2;
        }
        else
        {
            target[targetIndex] = sourceStart[i];
            ++i;
        }
        ++targetIndex;
    }

    target.resize(targetIndex);
}
