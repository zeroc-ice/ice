//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Protocol.h"
#include "Ice/LocalExceptions.h"

using namespace std;
using namespace Ice;

namespace IceInternal
{
    const byte magic[] = {byte{0x49}, byte{0x63}, byte{0x65}, byte{0x50}}; // 'I', 'c', 'e', 'P'

    const byte requestHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
        byte{requestMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0}, // Message size (placeholder)
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Request id (placeholder)
    };

    const byte requestBatchHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
        byte{requestBatchMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0}, // Message size (place holder)
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Number of requests in batch (placeholder)
    };

    const byte replyHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
        byte{replyMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Message size (placeholder)
    };

    void throwUnsupportedProtocolException(
        const char* f,
        int l,
        const Ice::ProtocolVersion& v,
        const Ice::ProtocolVersion& s)
    {
        throw UnsupportedProtocolException(f, l, "", v, s);
    }

    void throwUnsupportedEncodingException(
        const char* f,
        int l,
        const Ice::EncodingVersion& v,
        const Ice::EncodingVersion& s)
    {
        throw UnsupportedEncodingException(f, l, "", v, s);
    }
}
