// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * The InputStream class provides methods for decoding Slice encoded data from a stream of bytes.
         */
        class InputStream {
            /**
             * Constructs input stream that uses the communicator's encoding version 1.1, and the given buffer.
             *
             * @param communicator The communicator that provides the encoding version, the logger, and the value
             * factory manager.
             * @param buffer The buffer containing the encoded data.
             */
            constructor(communicator: Communicator, buffer: Uint8Array);

            /**
             * Constructs an input stream that uses the given encoding version, the given communicator, and the given buffer.
             *
             * @param communicator The communicator that provides the logger, and the value factory manager.
             * @param encoding The encoding version.
             * @param buffer The buffer containing the encoded data.
             */
            constructor(communicator: Communicator, encoding: EncodingVersion, buffer: Uint8Array);

            /**
             *  Releases any data retained by encapsulations.
             */
            clear(): void;

            /**
             * Swaps the contents of one stream with another.
             * @param other The other stream.
             */
            swap(other: InputStream): void;

            /**
             * Resizes the stream to a new size.
             *
             * @param sz The new size.
             */
            resize(sz: number): void;

            /**
             * Marks the start of a class instance.
             */
            startValue(): void;

            /**
             * Marks the end of a class instance.
             *
             * @returns A SlicedData object containing the preserved slices for unknown types.
             */
            endValue(): SlicedData;

            /**
             * Marks the start of an exception.
             */
            startException(): void;

            /**
             *
             * Marks the end of an exception.
             */
            endException(): void;

            /**
             * Reads the start of an encapsulation.
             *
             * @returns The encapsulation encoding version.
             */
            startEncapsulation(): EncodingVersion;

            /**
             * Reads the end of an encapsulation.
             */
            endEncapsulation(): void;

            /**
             * Skips an empty encapsulation.
             *
             * @returns The encoding version of the encapsulation.
             */
            skipEmptyEncapsulation(): EncodingVersion;

            /**
             * Reads and encapsulation from the stream.
             *
             * @returns a tuple containing:
             * - The encoding version of the encapsulation.
             * - The encapsulation data.
             */
            readEncapsulation(): [EncodingVersion, Uint8Array];

            /**
             * Gets the current encoding version of the encapsulation.
             *
             * @returns The encoding version.
             */
            getEncoding(): EncodingVersion;

            /**
             * Returns the size of the current encapsulation, excluding the encapsulation header.
             *
             * @returns The size of the encapsulated data.
             */
            getEncapsulationSize(): number;

            /**
             * Skips over an encapsulation.
             *
             * @returns The encoding version of the skipped encapsulation.
             */
            skipEncapsulation(): EncodingVersion;

            /**
             * Reads the start of a class instance or exception slice.
             *
             * @returns The Slice type ID for this slice.
             */
            startSlice(): string;

            /**
             * Indicates that the end of a class instance or exception slice has been reached.
             */
            endSlice(): void;

            /**
             * Skips over a class instance or exception slice.
             */
            skipSlice(): void;

            /**
             * Indicates that unmarshaling is complete, except for any class instances. The application must call this
             * method only if the stream actually contains class instances.
             */
            readPendingValues(): void;

            /**
             * Reads a size from the stream.
             *
             * @returns The size read from the stream.
             */
            readSize(): number;

            /**
             * Reads and validates a sequence size.
             *
             * @param minSize The minimum size of the sequence elements.
             * @returns The size read from the stream.
             */
            readAndCheckSeqSize(minSize: number): number;

            /**
             * Reads a blob of bytes from the stream.
             *
             * @param sz The number of bytes to read.
             * @returns The byte array read from the stream.
             */
            readBlob(sz: number): Uint8Array;

            /**
             * Determine if an optional value is available for reading.
             *
             * @param tag The tag associated with the value.
             * @param expectedFormat The optional format for the value.
             * @returns True if the optional value is present, false otherwise.
             */
            readOptional(tag: number, expectedFormat: OptionalFormat): boolean;

            /**
             * Reads a byte from the stream.
             *
             * @returns The byte read from the stream.
             */
            readByte(): number;

            /**
             * Reads a byte sequence from the stream.
             *
             * @returns The byte sequence read from the stream.
             */
            readByteSeq(): Uint8Array;

            /**
             * Reads a boolean from the stream.
             *
             * @returns The boolean read from the stream.
             */
            readBool(): boolean;

            /**
             * Reads a short from the stream.
             *
             * @returns The short read from the stream.
             */
            readShort(): number;

            /**
             * Reads an integer from the stream.
             *
             * @returns The integer read from the stream.
             */
            readInt(): number;

            /**
             * Reads a long from the stream.
             *
             * @returns The long read from the stream.
             */
            readLong(): Ice.Long;

            /**
             * Reads a float from the stream.
             *
             * @returns The float read from the stream.
             */
            readFloat(): number;

            /**
             * Reads a double from the stream.
             *
             * @returns The double read from the stream.
             */
            readDouble(): number;

            /**
             * Reads a string from the stream.
             *
             * @returns The string read from the stream.
             */
            readString(): string;

            /**
             * Reads a proxy from the stream. The stream must have been initialized with a communicator.
             *
             * @param type the type of the proxy to read.
             * @returns The proxy.
             * @throws {@link Ice.MarshalException} If the stream has not been initialized with a communicator and
             * cannot be used for unmarshaling proxies.
             */
            readProxy<T extends ObjectPrx>(type: new () => T): T;

            /**
             * Reads an optional proxy from the stream. The stream must have been initialized with a communicator.
             *
             * @param tag The numeric tag associated with the value.
             * @param type The type of the proxy to read.
             * @returns The proxy or undefined if the optional value is not present.
             * @throws {@link Ice.MarshalException} If the stream has not been initialized with a communicator and
             * cannot be used for unmarshaling proxies.
             */
            readOptionalProxy<T extends ObjectPrx>(tag: number, type: new () => T): T | undefined;

            /**
             * Reads an enumerated value.
             *
             * @param type The type of the enumeration.
             * @returns The enumerated value.
             */
            readEnum<T extends EnumBase>(type: new (name: string, value: number) => T): T;

            /**
             * Reads an enumerated value.
             *
             * @param tag The numeric tag associated with the value.
             * @param type The type of the enumeration.
             * @returns The enumerated value or undefined if the optional value is not present.
             */
            readOptionalEnum<T>(tag: number, type: new () => T): T | undefined;

            /**
             * Reads the index of a Slice value from the stream.
             *
             * @param cb The callback to notify the application when the extracted instance is available.
             * The stream extracts Slice values in stages. The Ice run time invokes the callback when the
             * corresponding instance has been fully unmarshaled.
             */
            readValue<T extends Value>(cb: (value: T) => void, type: new () => T): void;

            /**
             * Reads a user exception from the stream and throws it.
             */
            throwException(): void;

            /**
             * Skip the given number of bytes.
             *
             * @param size The number of bytes to skip.
             */
            skip(size: number): void;

            /**
             * Skip over a size value.
             */
            skipSize(): void;

            /**
             *  Returns whether the stream is empty.
             *
             * @returns True if the internal buffer has no data, false otherwise.
             */
            isEmpty(): boolean;

            /**
             * Gets or sets the stream position
             */
            pos: number;

            /**
             * Gets the size of the stream.
             */
            readonly size: number;
        }
    }
}
