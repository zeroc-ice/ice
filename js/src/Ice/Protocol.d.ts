//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        const Encoding_1_0:EncodingVersion;
        const Encoding_1_1:EncodingVersion;

        const Protocol_1_0:ProtocolVersion;

        class Protocol
        {
            //
            // Size of the Ice protocol header
            //
            // Magic number (4 bytes)
            // Protocol version major (Byte)
            // Protocol version minor (Byte)
            // Encoding version major (Byte)
            // Encoding version minor (Byte)
            // Message type (Byte)
            // Compression status (Byte)
            // Message size (Int)
            //
            static readonly headerSize:number;

            //
            // The magic number at the front of each message ['I', 'c', 'e', 'P']
            //
            static readonly magic:Uint8Array;

            //
            // The current Ice protocol and encoding version
            //
            static readonly protocolMajor:number;
            static readonly protocolMinor:number;
            static readonly protocolEncodingMajor:number;
            static readonly protocolEncodingMinor:number;

            static readonly encodingMajor:number;
            static readonly encodingMinor:number;

            //
            // The Ice protocol message types
            //
            static readonly requestMsg:number;
            static readonly requestBatchMsg:number;
            static readonly replyMsg:number;
            static readonly validateConnectionMsg:number;
            static readonly closeConnectionMsg:number;

            //
            // Reply status
            //
            static readonly replyOK:number;
            static readonly replyUserException:number;
            static readonly replyObjectNotExist:number;
            static readonly replyFacetNotExist:number;
            static readonly replyOperationNotExist:number;
            static readonly replyUnknownLocalException:number;
            static readonly replyUnknownUserException:number;
            static readonly replyUnknownException:number;

            static readonly requestHdr:Uint8Array;

            static readonly requestBatchHdr:Uint8Array;

            static readonly replyHdr:Uint8Array;

            static readonly currentProtocol:ProtocolVersion;
            static readonly currentProtocolEncoding:EncodingVersion;

            static currentEncoding:EncodingVersion;

            static checkSupportedProtocol(v:ProtocolVersion):void;
            static checkSupportedProtocolEncoding(v:EncodingVersion):void;
            static checkSupportedEncoding(version:EncodingVersion):void;

            //
            // Either return the given protocol if not compatible, or the greatest
            // supported protocol otherwise.
            //
            static getCompatibleProtocol(version:ProtocolVersion):ProtocolVersion;

            //
            // Either return the given encoding if not compatible, or the greatest
            // supported encoding otherwise.
            //
            static getCompatibleEncoding(version:EncodingVersion):EncodingVersion;

            static isSupported(version:ProtocolVersion, supported:ProtocolVersion):boolean;
            static isSupported(version:EncodingVersion, supported:EncodingVersion):boolean;

            static readonly OPTIONAL_END_MARKER:number;
            static readonly FLAG_HAS_TYPE_ID_STRING:number;
            static readonly FLAG_HAS_TYPE_ID_INDEX:number;
            static readonly FLAG_HAS_TYPE_ID_COMPACT:number;
            static readonly FLAG_HAS_OPTIONAL_MEMBERS:number;
            static readonly FLAG_HAS_INDIRECTION_TABLE:number;
            static readonly FLAG_HAS_SLICE_SIZE:number;
            static readonly FLAG_IS_LAST_SLICE:number;
        }

        /**
        * Converts a string to a protocol version.
        *
        * @param version The string to convert.
        *
        * @return The converted protocol version.
        **/
        function stringToProtocolVersion(version:string):ProtocolVersion;

        /**
        * Converts a string to an encoding version.
        *
        * @param version The string to convert.
        *
        * @return The converted object identity.
        **/
        function stringToEncodingVersion(version:string):EncodingVersion;

        /**
        * Converts a protocol version to a string.
        *
        * @param v The protocol version to convert.
        *
        * @return The converted string.
        **/
        function protocolVersionToString(version:ProtocolVersion):string;

        /**
         * Converts an encoding version to a string.
         *
         * @param v The encoding version to convert.
         *
         * @return The converted string.
         **/
        function encodingVersionToString(version:EncodingVersion):string;
    }
}
