//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Current");
require("../Ice/Debug");
require("../Ice/Identity");
require("../Ice/Protocol");
require("../Ice/StringUtil");

//
// Local aliases.
//
const Debug = Ice.Debug;
const Identity = Ice.Identity;
const OperationMode = Ice.OperationMode;
const Protocol = Ice.Protocol;
const StringUtil = Ice.StringUtil;

const slicingIds = new Map();

function printIdentityFacetOperation(s, stream)
{
    let toStringMode = Ice.ToStringMode.Unicode;
    if(stream.instance !== null)
    {
        toStringMode = stream.instance.toStringMode();
    }

    const identity = new Identity();
    identity._read(stream);
    s.push("\nidentity = " + Ice.identityToString(identity, toStringMode));

    const facet = Ice.StringSeqHelper.read(stream);
    s.push("\nfacet = ");
    if(facet.length > 0)
    {
        s.push(StringUtil.escapeString(facet[0], "", toStringMode));
    }

    const operation = stream.readString();
    s.push("\noperation = " + operation);
}

function printRequest(s, stream)
{
    const requestId = stream.readInt();
    s.push("\nrequest id = " + requestId);
    if(requestId === 0)
    {
        s.push(" (oneway)");
    }

    printRequestHeader(s, stream);
}

function printBatchRequest(s, stream)
{
    const batchRequestNum = stream.readInt();
    s.push("\nnumber of requests = " + batchRequestNum);

    for(let i = 0; i < batchRequestNum; ++i)
    {
        s.push("\nrequest #" + i + ':');
        printRequestHeader(s, stream);
    }
}

function printReply(s, stream)
{
    const requestId = stream.readInt();
    s.push("\nrequest id = " + requestId);

    const replyStatus = stream.readByte();
    s.push("\nreply status = " + replyStatus + ' ');

    switch(replyStatus)
    {
    case Protocol.replyOK:
    {
        s.push("(ok)");
        break;
    }

    case Protocol.replyUserException:
    {
        s.push("(user exception)");
        break;
    }

    case Protocol.replyObjectNotExist:
    case Protocol.replyFacetNotExist:
    case Protocol.replyOperationNotExist:
    {
        switch(replyStatus)
        {
        case Protocol.replyObjectNotExist:
        {
            s.push("(object not exist)");
            break;
        }

        case Protocol.replyFacetNotExist:
        {
            s.push("(facet not exist)");
            break;
        }

        case Protocol.replyOperationNotExist:
        {
            s.push("(operation not exist)");
            break;
        }

        default:
        {
            Debug.assert(false);
            break;
        }
        }

        printIdentityFacetOperation(s, stream);
        break;
    }

    case Protocol.replyUnknownException:
    case Protocol.replyUnknownLocalException:
    case Protocol.replyUnknownUserException:
    {
        switch(replyStatus)
        {
        case Protocol.replyUnknownException:
        {
            s.push("(unknown exception)");
            break;
        }

        case Protocol.replyUnknownLocalException:
        {
            s.push("(unknown local exception)");
            break;
        }

        case Protocol.replyUnknownUserException:
        {
            s.push("(unknown user exception)");
            break;
        }

        default:
        {
            Debug.assert(false);
            break;
        }
        }

        const unknown = stream.readString();
        s.push("\nunknown = " + unknown);
        break;
    }

    default:
    {
        s.push("(unknown)");
        break;
    }
    }

    if(replyStatus === Protocol.replyOK || replyStatus === Protocol.replyUserException)
    {
        const ver = stream.skipEncapsulation();
        if(!ver.equals(Ice.Encoding_1_0))
        {
            s.push("\nencoding = ");
            s.push(Ice.encodingVersionToString(ver));
        }
    }
}

function printRequestHeader(s, stream)
{
    printIdentityFacetOperation(s, stream);

    const mode = stream.readByte();
    s.push("\nmode = " + mode + ' ');
    switch(OperationMode.valueOf(mode))
    {
        case OperationMode.Normal:
        {
            s.push("(normal)");
            break;
        }

        case OperationMode.Nonmutating:
        {
            s.push("(nonmutating)");
            break;
        }

        case OperationMode.Idempotent:
        {
            s.push("(idempotent)");
            break;
        }

        default:
        {
            s.push("(unknown)");
            break;
        }
    }

    let sz = stream.readSize();
    s.push("\ncontext = ");
    while(sz-- > 0)
    {
        const key = stream.readString();
        const value = stream.readString();
        s.push(key + '/' + value);
        if(sz > 0)
        {
            s.push(", ");
        }
    }

    const ver = stream.skipEncapsulation();
    if(!ver.equals(Ice.Encoding_1_0))
    {
        s.push("\nencoding = ");
        s.push(Ice.encodingVersionToString(ver));
    }
}

function printHeader(s, stream)
{
    stream.readByte(); // Don't bother printing the magic number
    stream.readByte();
    stream.readByte();
    stream.readByte();

//        const pMajor = stream.readByte();
//        const pMinor = stream.readByte();
//        s.push("\nprotocol version = " + pMajor + "." + pMinor);
    stream.readByte(); // major
    stream.readByte(); // minor

//        const eMajor = stream.readByte();
//        const eMinor = stream.readByte();
//        s.push("\nencoding version = " + eMajor + "." + eMinor);
    stream.readByte(); // major
    stream.readByte(); // minor

    const type = stream.readByte();

    s.push("\nmessage type = " + type + " (" + getMessageTypeAsString(type) + ')');
    const compress = stream.readByte();
    s.push("\ncompression status = " + compress + ' ');
    switch(compress)
    {
        case 0:
        {
            s.push("(not compressed; do not compress response, if any)");
            break;
        }

        case 1:
        {
            s.push("(not compressed; compress response, if any)");
            break;
        }

        case 2:
        {
            s.push("(compressed; compress response, if any)");
            break;
        }

        default:
        {
            s.push("(unknown)");
            break;
        }
    }

    const size = stream.readInt();
    s.push("\nmessage size = " + size);
    return type;
}

function printMessage(s, stream)
{
    const type = printHeader(s, stream);

    switch(type)
    {
    case Protocol.closeConnectionMsg:
    case Protocol.validateConnectionMsg:
    {
        // We're done.
        break;
    }

    case Protocol.requestMsg:
    {
        printRequest(s, stream);
        break;
    }

    case Protocol.requestBatchMsg:
    {
        printBatchRequest(s, stream);
        break;
    }

    case Protocol.replyMsg:
    {
        printReply(s, stream);
        break;
    }

    default:
    {
        break;
    }
    }

    return type;
}

function getMessageTypeAsString(type)
{
    switch(type)
    {
    case Protocol.requestMsg:
        return "request";
    case Protocol.requestBatchMsg:
        return "batch request";
    case Protocol.replyMsg:
        return "reply";
    case Protocol.closeConnectionMsg:
        return "close connection";
    case Protocol.validateConnectionMsg:
        return "validate connection";
    default:
        return "unknown";
    }
}

class TraceUtil
{
    static traceSlicing(kind, typeId, slicingCat, logger)
    {
        if(!slicingIds.has(typeId))
        {
            logger.trace(slicingCat, `unknown ${kind} type \`${typeId}'`);
            slicingIds.set(typeId, 1);
        }
    }

    static traceSend(stream, logger, traceLevels)
    {
        if(traceLevels.protocol >= 1)
        {
            const p = stream.pos;
            const is = new Ice.InputStream(stream.instance, stream.getEncoding(), stream.buffer);
            is.pos = 0;

            const s = [];
            const type = printMessage(s, is);

            logger.trace(traceLevels.protocolCat, "sending " + getMessageTypeAsString(type) + " " + s.join(""));

            stream.pos = p;
        }
    }

    static traceRecv(stream, logger, traceLevels)
    {
        if(traceLevels.protocol >= 1)
        {
            const p = stream.pos;
            stream.pos = 0;

            const s = [];
            const type = printMessage(s, stream);

            logger.trace(traceLevels.protocolCat, "received " + getMessageTypeAsString(type) + " " + s.join(""));

            stream.pos = p;
        }
    }

    static traceOut(heading, stream, logger, traceLevels)
    {
        if(traceLevels.protocol >= 1)
        {
            const p = stream.pos;
            const is = new Ice.InputStream(stream.instance, stream.getEncoding(), stream.buffer);
            is.pos = 0;

            const s = [];
            s.push(heading);
            printMessage(s, is);

            logger.trace(traceLevels.protocolCat, s.join(""));
            stream.pos = p;
        }
    }

    static traceIn(heading, stream, logger, traceLevels)
    {
        if(traceLevels.protocol >= 1)
        {
            const p = stream.pos;
            stream.pos = 0;

            const s = [];
            s.push(heading);
            printMessage(s, stream);

            logger.trace(traceLevels.protocolCat, s.join(""));
            stream.pos = p;
        }
    }

    static dumpStream(stream)
    {
        const pos = stream.pos;
        stream.pos = 0;

        const data = stream.readBlob(stream.size());
        TraceUtil.dumpOctets(data);

        stream.pos = pos;
    }

    static dumpOctets(data)
    {
        const inc = 8;
        const buf = [];

        for(let i = 0; i < data.length; i += inc)
        {
            for(let j = i; j - i < inc; j++)
            {
                if(j < data.length)
                {
                    let n = data[j];
                    if(n < 0)
                    {
                        n += 256;
                    }
                    let s;
                    if(n < 10)
                    {
                        s = "  " + n;
                    }
                    else if(n < 100)
                    {
                        s = " " + n;
                    }
                    else
                    {
                        s = String(n);
                    }
                    buf.push(s + " ");
                }
                else
                {
                    buf.push("    ");
                }
            }

            buf.push('"');

            for(let j = i; j < data.length && j - i < inc; j++)
            {
                if(data[j] >= 32 && data[j] < 127)
                {
                    buf.push(String.fromCharCode(data[j]));
                }
                else
                {
                    buf.push('.');
                }
            }

            buf.push("\"\n");
        }

        console.log(buf.join(""));
    }
}

Ice.TraceUtil = TraceUtil;
module.exports.Ice = Ice;
