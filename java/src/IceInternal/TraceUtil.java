// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

final class TraceUtil
{
    static void
    traceHeader(String heading, Ice.Stream stream, Ice.Logger logger,
                TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            // TODO: stream stuff
            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, stream);
            logger.trace(tl.protocolCat, s.toString());
        }
    }

    static void
    traceRequest(String heading, Ice.Stream stream, Ice.Logger logger,
                 TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            // TODO: stream stuff
            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, stream);
            int requestId = stream.readInt();
            s.write("\nrequest id = " + requestId);
            if (requestId == 0)
            {
                s.write(" (oneway)");
            }
            String identity = stream.readString();
            s.write("\nidentity = " + identity);
            String facet = stream.readString();
            s.write("\nfacet = " + facet);
            String operation = stream.readString();
            s.write("\noperation name = " + operation);
            logger.trace(tl.protocolCat, s.toString());
        }
    }

    static void
    traceBatchRequest(String heading, Ice.Stream stream, Ice.Logger logger,
                      TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            // TODO: stream stuff
            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, stream);
            int cnt = 0;
            while (true /* stream.i != stream.b.end() */ )
            {
                s.write("\nrequest #" + cnt + ':');
                cnt++;
                // TODO
                stream.startReadEncaps();
                String identity = stream.readString();
                s.write("\nidentity = " + identity);
                String facet = stream.readString();
                s.write("\nfacet = " + facet);
                String operation = stream.readString();
                s.write("\noperation name = " + operation);
                // TODO
                stream.skipEncaps();
            }
            logger.trace(tl.protocolCat, s.toString());
        }
    }

    static void
    traceReply(String heading, Ice.Stream stream, Ice.Logger logger,
               TraceLevels tl)
    {
        if (tl.protocol >= 1)
        {
            // TODO: stream stuff
            java.io.StringWriter s = new java.io.StringWriter();
            s.write(heading);
            printHeader(s, stream);
            int requestId = stream.readInt();
            s.write("\nrequest id = " + requestId);
            byte status = stream.readByte();
            s.write("\nreply status = " + (int)status + ' ');
            switch (status)
            {
                case DispatchStatus._DispatchOK:
                {
                    s.write("(ok)");
                    break;
                }
                case DispatchStatus._DispatchUserException:
                {
                    s.write("(user exception)");
                    break;
                }
                case DispatchStatus._DispatchLocationForward:
                {
                    s.write("(location forward)");
                    break;
                }
                case DispatchStatus._DispatchObjectNotExist:
                {
                    s.write("(object not exist)");
                    break;
                }
                case DispatchStatus._DispatchOperationNotExist:
                {
                    s.write("(operation not exist)");
                    break;
                }
                case DispatchStatus._DispatchUnknownLocalException:
                {
                    s.write("(unknown local exception)");
                    break;
                }
                case DispatchStatus._DispatchUnknownException:
                {
                    s.write("(unknown exception)");
                    break;
                }
                default:
                {
                    s.write("(unknown)");
                    break;
                }
            }
            logger.trace(tl.protocolCat, s.toString());
        }
    }
}
