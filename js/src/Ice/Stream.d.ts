//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        class ByteHelper
        {
            static validate(v:number):boolean;
        }

        class ShortHelper
        {
            static validate(v:number):boolean;
        }

        class IntHelper
        {
            static validate(v:number):boolean;
        }

        class FloatHelper
        {
            static validate(v:number):boolean;
        }

        class DoubleHelper
        {
            static validate(v:number):boolean;
        }

        class LongHelper
        {
            static validate(v:Long):boolean;
        }

        class InputStream
        {
            constructor();
            constructor(buffer:Uint8Array);
            constructor(communicator:Communicator);
            constructor(communicator:Communicator, buffer:Uint8Array);
            constructor(encoding:EncodingVersion);
            constructor(encoding:EncodingVersion, buffer:Uint8Array);
            constructor(communicator:Communicator, encoding:EncodingVersion);
            constructor(communicator:Communicator, encoding:EncodingVersion, buffer:Uint8Array);

            //
            // This function allows this object to be reused, rather than reallocated.
            //
            reset():void;
            clear():void;
            swap(other:InputStream):void;
            resetEncapsulation():void;
            resize(sz:number):void;
            startValue():void;
            endValue(preserve:boolean):SlicedData;
            startException():void;
            endException(preserve:boolean):SlicedData;
            startEncapsulation():EncodingVersion;
            endEncapsulation():void;
            skipEmptyEncapsulation():EncodingVersion;
            readEncapsulation(encoding:EncodingVersion):Uint8Array;
            getEncoding():EncodingVersion;
            getEncapsulationSize():number;
            skipEncapsulation():EncodingVersion;
            startSlice():string; // Returns type ID of next slice
            endSlice():void;
            skipSlice():void;
            readPendingValues():void;
            readSize():number;
            readAndCheckSeqSize(minSize:number):number;
            readBlob(sz:number):Uint8Array;
            readOptional(tag:number, expectedFormat:OptionalFormat):boolean;
            readByte():number;
            readByteSeq():Uint8Array;
            readBool():boolean;
            readShort():number;
            readInt():number;
            readLong():Ice.Long;
            readFloat():number;
            readDouble():number;
            readString():string;

            readProxy<T extends ObjectPrx>(type:new()=>T):T;
            readOptionalProxy<T extends ObjectPrx>(tag:number, type:new()=>T):T|undefined;
            readEnum<T>(type:new()=>T):T;
            readOptionalEnum<T>(tag:number, type:new()=>T):T|undefined;
            readValue<T extends Value>(cb:(value:T) => void, type:new()=>T):void;
            readOptionalValue<T extends Value>(tag:number, cb:(value:T) => void, type:new()=>T):void;
            throwException():void;
            skip(size:number):void;
            skipSize():void;
            isEmpty():boolean;
            expand(n:number):void;

            //
            // Sets the value factory manager to use when marshaling value instances. If the stream
            // was initialized with a communicator, the communicator's value factory manager will
            // be used by default.
            //
            valueFactoryManager:ValueFactoryManager;

            //
            // Sets the logger to use when logging trace messages. If the stream
            // was initialized with a communicator, the communicator's logger will
            // be used by default.
            //
            logger:Logger;

            //
            // Sets the compact ID resolver to use when unmarshaling value and exception
            // instances. If the stream was initialized with a communicator, the communicator's
            // resolver will be used by default.
            //
            compactIdResolver:(compactID:number) => string;

            //
            // Determines the behavior of the stream when extracting instances of Slice classes.
            // A instance is "sliced" when a factory cannot be found for a Slice type ID.
            // The stream's default behavior is to slice instances.
            //
            // If slicing is disabled and the stream encounters a Slice type ID
            // during decoding for which no value factory is installed, it raises
            // NoValueFactoryException.
            //
            sliceValues:boolean;

            //
            // Determines whether the stream logs messages about slicing instances of Slice values.
            //
            traceSlicing:boolean;

            pos:number;

            readonly size:number;
            readonly buffer:Uint8Array;
        }

        class OutputStream
        {
            constructor(communicator?:Ice.Communicator, encoding?:EncodingVersion, buffer?:Uint8Array);

            //
            // This function allows this object to be reused, rather than reallocated.
            //
            reset():void;
            clear():void;
            finished():Uint8Array;
            swap(other:OutputStream):void;
            resetEncapsulation():void;
            resize(sz:number):void;
            prepareWrite():Uint8Array;
            startValue(data:SlicedData):void;
            endValue():void;
            startException(data:SlicedData):void;
            endException():void;
            startEncapsulation():void;
            startEncapsulation(encoding:EncodingVersion, format:FormatType):void;
            endEncapsulation():void;
            writeEmptyEncapsulation(encoding:EncodingVersion):void;
            writeEncapsulation(buff:Uint8Array):void;
            getEncoding():EncodingVersion;
            startSlice(typeId:string, compactId:number, last:boolean):void;
            endSlice():void;
            writePendingValues():void;
            writeSize(v:number):void;
            startSize():number;
            endSize(pos:number):void;
            writeBlob(v:Uint8Array):void;
            // Read/write format and tag for optionals
            writeOptional(tag:number, format:OptionalFormat):void;
            writeByte(v:number):void;
            writeByteSeq(v:Uint8Array):void;
            writeBool(v:boolean):void;
            writeShort(v:number):void;
            writeInt(v:number):void;
            writeLong(v:Ice.Long):void;
            writeFloat(v:number):void;
            writeDouble(v:number):void;
            writeString(v:string):void;
            writeProxy(v:ObjectPrx):void;
            writeOptionalProxy(tag:number, v?:ObjectPrx|undefined):void;

            /// TODO use a base enum type
            writeEnum(v:any):void;

            writeValue(v:Ice.Value):void;
            writeOptionalValue(tag:number, v?:Ice.Value|undefined):void;
            writeException(e:UserException):void;
            writeUserException(e:UserException):void;

            isEmpty():boolean;

            expand(n:number):void;

            //
            // Sets the encoding format for class and exception instances.
            //
            format:FormatType;
            pos:number;
            readonly size:number;
            readonly buffer:Uint8Array;
        }
    }
}
