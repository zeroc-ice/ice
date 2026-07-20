// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents a byte buffer used for marshaling data using the Slice encoding.
         */
        class OutputStream {
            /**
             * Constructs an OutputStream using the encoding and format provided by the communicator.
             *
             * @param communicator The communicator.
             */
            constructor(communicator: Communicator);

            /**
             * Constructs an empty output stream.
             *
             * @param encoding The encoding version to use. `null` is equivalent to encoding 1.1.
             * @param format The class format to use. `null` is equivalent to `FormatType.CompactFormat`.
             */
            constructor(encoding?: EncodingVersion, format?: FormatType);

            /**
             *  Releases any data retained by encapsulations.
             */
            clear(): void;

            /**
             * Indicates that marshaling is complete.
             * @returns The `Uint8Array` containing the encoded request or reply
             */
            finished(): Uint8Array;

            /**
             * Swaps the contents of this stream with another.
             *
             * @param other The other stream.
             */
            swap(other: OutputStream): void;

            /**
             * Resizes the stream to a new size.
             *
             * @param sz The new size of the stream in bytes.
             */
            resize(sz: number): void;

            /**
             * Marks the start of a class instance.
             *
             * @param slicedData Preserved slices for this instance, or `null`. These slices are marshaled with the
             * instance only when this stream uses the sliced format; otherwise they are ignored.
             */
            startValue(slicedData: SlicedData | null): void;

            /**
             * Marks the end of a class instance.
             */
            endValue(): void;

            /**
             * Marks the start of an exception.
             */
            startException(): void;

            /**
             * Marks the end of an exception.
             */
            endException(): void;

            /**
             * Writes the start of an encapsulation. A nested encapsulation uses the encoding version and class format
             * of the enclosing encapsulation; a top-level encapsulation uses the stream's encoding version and class
             * format.
             */
            startEncapsulation(): void;

            /**
             * Writes the start of an encapsulation using the specified encoding version and class format.
             *
             * @param encoding The encoding version of the encapsulation.
             * @param format The class format of the encapsulation. If not specified, the OutputStream's class format
             * is used.
             */
            startEncapsulation(encoding: EncodingVersion, format?: FormatType): void;

            /**
             * Ends the current encapsulation.
             */
            endEncapsulation(): void;

            /**
             * Writes an empty encapsulation using the given encoding version.
             *
             * @param encoding The encoding version of the encapsulation.
             */
            writeEmptyEncapsulation(encoding: EncodingVersion): void;

            /**
             * Writes a pre-encoded encapsulation to the stream.
             *
             * @param buff The encapsulation data.
             */
            writeEncapsulation(buff: Uint8Array): void;

            /**
             * Gets the current encoding version.
             *
             * @returns The encoding version.
             */
            getEncoding(): EncodingVersion;

            /**
             * Marks the start of a new slice for a class instance or user exception.
             *
             * @param typeId The Slice type ID corresponding to this slice.
             * @param compactId The Slice compact type ID corresponding to this slice or -1 if no compact ID is defined
             * for the type ID.
             * @param last True if this is the last slice, false otherwise.
             */
            startSlice(typeId: string, compactId: number, last: boolean): void;

            /**
             * Marks the end of a slice for a class instance or user exception.
             */
            endSlice(): void;

            /**
             * Encodes the state of class instances whose insertion was delayed during a previous call to
             * {@link OutputStream#writeValue}. This function must be called only once. For backward compatibility with
             * encoding version 1.0, this function must be called only when non-optional fields or parameters use class
             * types.
             */
            writePendingValues(): void;

            /**
             * Writes a size to the stream.
             *
             * @param v The size to write.
             */
            writeSize(v: number): void;

            /**
             * Returns the current position and allocates four bytes for a fixed-length (32-bit) size value.
             *
             * @returns The current position.
             */
            startSize(): number;

            /**
             * Computes the amount of data written since the previous call to startSize and writes that value at the
             * saved position.
             *
             * @param pos The saved position.
             */
            endSize(pos: number): void;

            /**
             * Writes a blob of bytes to the stream.
             *
             * @param v The blob of bytes to write.
             */
            writeBlob(v: Uint8Array): void;

            /**
             *
             * Write the header information for an optional value.
             *
             * @param tag The numeric tag associated with the value.
             * @param format The optional format of the value.
             * @returns `true` if the current encoding version supports optional values, `false` otherwise. If `true`,
             * the data associated with the optional value must be written next.
             */
            writeOptional(tag: number, format: OptionalFormat): boolean;

            /**
             * Writes a byte to the stream.
             *
             * @param v The byte to write to the stream.
             */
            writeByte(v: number): void;

            /**
             *
             * Writes a byte sequence to the stream.
             *
             * @param v The byte sequence to write to the stream. Passing `null` causes an empty sequence to be written
             *          to the stream.
             */
            writeByteSeq(v?: Uint8Array): void;

            /**
             * Writes a boolean to the stream.
             *
             * @param v The boolean to write to the stream.
             */
            writeBool(v: boolean): void;

            /**
             * Writes a short to the stream.
             *
             * @param v The short to write to the stream.
             */
            writeShort(v: number): void;

            /**
             * Writes an integer to the stream.
             *
             * @param v The integer to write to the stream.
             */
            writeInt(v: number): void;

            /**
             * Writes a long to the stream.
             *
             * @param v The long to write to the stream.
             */
            writeLong(v: bigint | number): void;

            /**
             * Writes a float to the stream.
             *
             * @param v The float to write to the stream.
             */
            writeFloat(v: number): void;

            /**
             * Writes a double to the stream.
             *
             * @param v The double to write to the stream.
             */
            writeDouble(v: number): void;

            /**
             * Writes a string to the stream.
             *
             * @param v The string to write to the stream. Passing `null` causes an empty string to be written to the
             *          stream.
             */
            writeString(v: string | null): void;

            /**
             * Writes a proxy to the stream.
             *
             * @param v The proxy to write.
             */
            writeProxy(v: ObjectPrx | null): void;

            /**
             * Writes an optional proxy to the stream.
             *
             * @param tag The optional tag.
             * @param v The proxy to write, or `undefined` if the optional proxy is not present.
             */
            writeOptionalProxy(tag: number, v?: ObjectPrx | undefined): void;

            /**
             * Writes an enumerated value.
             *
             * @param v The enumerator to write to the stream.
             */
            writeEnum(v: EnumBase): void;

            /**
             * Writes a class instance to the stream.
             *
             * @param v The value to write. With the 1.0 encoding, this method writes an index and the state of the
             * value is marshaled when {@link OutputStream#writePendingValues} is called; with the 1.1 encoding, the
             * state is marshaled eagerly, without waiting for {@link OutputStream#writePendingValues}.
             */
            writeValue(v: Ice.Value): void;

            /**
             * Writes a user exception to the stream.
             *
             * @param exception The user exception to write.
             */
            writeException(exception: UserException): void;

            /**
             *  Returns whether the stream is empty.
             *
             * @returns True if no data has been written yet, false otherwise.
             */
            isEmpty(): boolean;

            /**
             * Expand the stream to accept more data.
             *
             * @param n The number of bytes to accommodate in the stream.
             */
            expand(n: number): void;

            /**
             * Gets or sets the stream position.
             */
            pos: number;

            /**
             * Gets the size of the stream.
             */
            readonly size: number;
        }
    }
}
