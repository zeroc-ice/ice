// Copyright (c) ZeroC, Inc.

import Foundation
import IceImpl

/// Stream class to read (unmarshal) Slice types from a sequence of bytes.
public final class InputStream {
    let data: Data
    let communicator: Communicator

    let sliceLoader: SliceLoader

    private(set) var pos: Int = 0
    private let encoding: EncodingVersion
    fileprivate let acceptClassCycles: Bool

    private var encaps: Encaps!

    private var startSeq: Int32 = -1
    private var minSeqSize: Int32 = 0
    private let classGraphDepthMax: Int32

    private let endOfBufferMessage = "attempting to unmarshal past the end of the buffer"

    private var remaining: Int {
        return data.count - pos
    }

    var currentEncoding: EncodingVersion {
        return encaps != nil ? encaps.encoding : encoding
    }

    public convenience init(communicator: Communicator, bytes: Data) {
        let encoding = (communicator as! CommunicatorI).defaultsAndOverrides.defaultEncoding
        self.init(communicator: communicator, encoding: encoding, bytes: bytes)
    }

    public required init(
        communicator: Communicator,
        encoding: EncodingVersion,
        bytes: Data
    ) {
        data = bytes
        self.communicator = communicator
        sliceLoader = (communicator as! CommunicatorI).initData.sliceLoader!
        self.encoding = encoding
        classGraphDepthMax = (communicator as! CommunicatorI).classGraphDepthMax
        acceptClassCycles = (communicator as! CommunicatorI).acceptClassCycles
    }

    /// Reads an encapsulation from the stream.
    ///
    /// - returns: `(bytes: Data, encoding: EncodingVersion)` The encapsulation.
    public func readEncapsulation() throws -> (bytes: Data, encoding: EncodingVersion) {
        let sz: Int32 = try read()
        if sz < 6 {
            throw MarshalException(endOfBufferMessage)
        }

        if sz - 4 > remaining {
            throw MarshalException(endOfBufferMessage)
        }

        let encoding: EncodingVersion = try read()
        try changePos(offset: -6)

        let bytes = data[pos..<pos + Int(sz)]
        return (bytes, encoding)
    }

    /// Reads the start of an encapsulation.
    ///
    /// - returns: `Ice.EncodingVersion` - The encapsulation encoding version.
    @discardableResult
    public func startEncapsulation() throws -> EncodingVersion {
        precondition(encaps == nil, "Nested or sequential encapsulations are not supported")

        let start = pos
        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        let sz: Int32 = try read()

        if sz < 6 {
            throw MarshalException(endOfBufferMessage)
        }
        if sz - 4 > remaining {
            throw MarshalException(endOfBufferMessage)
        }

        let encoding: EncodingVersion = try read()

        try checkSupportedEncoding(encoding)

        encaps = Encaps(start: start, size: Int(sz), encoding: encoding)

        return encoding
    }

    /// Ends the previous encapsulation.
    public func endEncapsulation() throws {
        if !encaps.encoding_1_0 {
            try skipOptionals()
            if pos != encaps.start + encaps.sz {
                throw MarshalException("buffer size does not match decoded encapsulation size")
            }
        } else if pos != encaps.start + encaps.sz {
            if pos + 1 != encaps.start + encaps.sz {
                throw MarshalException("buffer size does not match decoded encapsulation size")
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //
            try skip(1)
        }
    }

    /// Skips an empty encapsulation.
    ///
    /// - returns: `Ice.EncodingVersion` - The encapsulation's encoding version.
    @discardableResult
    public func skipEmptyEncapsulation() throws -> EncodingVersion {
        let sz: Int32 = try read()

        if sz < 6 {
            throw MarshalException("invalid encapsulation size")
        }

        if sz - 4 > remaining {
            throw MarshalException(endOfBufferMessage)
        }

        let encoding: EncodingVersion = try read()
        try checkSupportedEncoding(encoding)  // Make sure the encoding is supported.

        if encoding == Encoding_1_0 {
            if sz != 6 {
                throw MarshalException("invalid encapsulation size")
            }
        } else {
            //
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            //
            try skip(sz - 6)
        }

        return encoding
    }

    /// Skips over an encapsulation.
    ///
    /// - returns: `Ice.EncodingVersion` - The encoding version of the skipped encapsulation.
    func skipEncapsulation() throws -> EncodingVersion {
        let sz: Int32 = try read()

        if sz < 6 {
            throw MarshalException("invalid encapsulation size")
        }

        let encodingVersion: EncodingVersion = try read()
        try changePos(offset: Int(sz) - 6)
        return encodingVersion
    }

    /// Reads the start of a class instance or exception slice.
    public func startSlice() throws {
        precondition(encaps.decoder != nil)
        try encaps.decoder.startSlice()
    }

    /// Indicates that the end of a class instance or exception slice has been reached.
    public func endSlice() throws {
        precondition(encaps.decoder != nil)
        try encaps.decoder.endSlice()
    }

    /// Skips over a class instance or exception slice.
    public func skipSlice() throws {
        precondition(encaps.decoder != nil)
        try encaps.decoder.skipSlice()
    }

    /// Indicates that unmarshaling is complete, except for any class instances. The application must call this method
    /// only if the stream actually contains class instances. Calling `readPendingValues` triggers the
    /// calls to consumers provided with {@link #readValue} to inform the application that unmarshaling of an instance
    /// is complete.
    public func readPendingValues() throws {
        if encaps.decoder != nil {
            try encaps.decoder.readPendingValues()
        } else if encaps.encoding_1_0 {
            //
            // If using the 1.0 encoding and no instances were read, we
            // still read an empty sequence of pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            //
            try skipSize()
        }
    }

    /// Extracts a user exception from the stream and throws it.
    public func throwException() throws {
        initEncaps()
        try encaps.decoder.throwException()
    }

    func skipOptional(format: OptionalFormat) throws {
        switch format {
        case .F1:
            try skip(1)
        case .F2:
            try skip(2)
        case .F4:
            try skip(4)
        case .F8:
            try skip(8)
        case .Size:
            try skipSize()
        case .VSize:
            try skip(readSize())
        case .FSize:
            try skip(read())
        case .Class:
            throw MarshalException("cannot skip an optional class")
        }
    }

    func skipOptionals() throws {
        //
        // Skip remaining un-read optional members.
        //
        while true {
            if pos >= encaps.start + encaps.sz {
                return  // End of encapsulation also indicates end of optionals.
            }

            let v: UInt8 = try read()
            if v == SliceFlags.OPTIONAL_END_MARKER.rawValue {
                return
            }

            // Read first 3 bits.
            guard let format = OptionalFormat(rawValue: v & 0x07) else {
                preconditionFailure("invalid optional format")
            }

            if v >> 3 == 30 {
                try skipSize()
            }

            try skipOptional(format: format)
        }
    }

    // Reset the InputStream to prepare for retry
    func startOver() {
        pos = 0
        encaps = nil
    }

    private func changePos(offset: Int) throws {
        precondition(pos + offset >= 0, "Negative position")

        guard offset <= remaining else {
            throw MarshalException(endOfBufferMessage)
        }
        pos += offset
    }

    /// Skips the given number of bytes.
    ///
    /// - parameter count: `Int` - The number of bytes to skip.
    public func skip(_ count: Int) throws {
        precondition(count >= 0, "skip count is negative")
        try changePos(offset: count)
    }

    /// Skips the given number of bytes.
    ///
    /// - parameter count: `Int32` - The number of bytes to skip.
    public func skip(_ count: Int32) throws {
        try changePos(offset: Int(count))
    }

    /// Skip over a size value.
    public func skipSize() throws {
        let b: UInt8 = try read()
        if b == 255 {
            try skip(4)
        }
    }

    /// Marks the start of a class instance.
    public func startValue() {
        precondition(encaps.decoder != nil)
        encaps.decoder.startInstance(type: .ValueSlice)
    }

    /// Marks the end of a class instance.
    ///
    /// - returns: `Ice.SlicedData` - A SlicedData object containing the preserved slices for unknown types.
    @discardableResult
    public func endValue() throws -> SlicedData? {
        precondition(encaps.decoder != nil)
        return try encaps.decoder.endInstance()
    }

    /// Marks the start of a user exception.
    public func startException() {
        precondition(encaps.decoder != nil)
        encaps.decoder.startInstance(type: .ExceptionSlice)
    }

    /// Marks the end of a user exception.
    public func endException() throws {
        precondition(encaps.decoder != nil)
        _ = try encaps.decoder.endInstance()
    }

    func initEncaps() {
        if encaps == nil {
            encaps = Encaps(start: 0, size: data.count, encoding: encoding)
        }
        if encaps.decoder == nil {  // Lazy initialization
            if encaps.encoding_1_0 {
                encaps.decoder = EncapsDecoder10(
                    stream: self,
                    classGraphDepthMax: classGraphDepthMax)
            } else {
                encaps.decoder = EncapsDecoder11(
                    stream: self,
                    classGraphDepthMax: classGraphDepthMax)
            }
        }
    }

    fileprivate func traceSkipSlice(typeId: String, sliceType: SliceType) {
        guard (communicator as! CommunicatorI).traceSlicing else {
            return
        }

        ICETraceUtil.traceSlicing(
            kind: sliceType == SliceType.ExceptionSlice ? "exception" : "object",
            typeId: typeId,
            slicingCat: "Slicing",
            logger: LoggerWrapper(handle: communicator.getLogger()))
    }

    static func throwUOE(expectedType: Value.Type, v: Value) throws {
        if let usv = v as? UnknownSlicedValue {
            throw MarshalException(
                "The Slice loader did not find a class for type ID '\(usv.ice_id())'.")
        }

        throw MarshalException(
            "Failed to unmarshal class with type ID '\(expectedType.ice_staticId())': the Slice loader returned a class with type ID '\(v.ice_id())'."
        )
    }
}

extension InputStream {
    /// Reads a numeric value from the stream.
    ///
    /// - returns: `Element` - The numeric value read from the stream.
    public func read<Element>() throws -> Element where Element: StreamableNumeric {
        let size = MemoryLayout<Element>.size
        guard size <= remaining else {
            throw MarshalException(endOfBufferMessage)
        }

        var value: Element = 0
        // We assume a little-endian platform
        withUnsafeMutablePointer(to: &value) { ptr in
            let buf = UnsafeMutableBufferPointer(start: ptr, count: 1)
            self.data.copyBytes(to: buf, from: self.pos..<self.pos + size)
        }
        pos += size
        return value
    }

    /// Reads an optional numeric value from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `Element?` - The optional numeric value read from the stream.
    public func read<Element>(tag: Int32) throws -> Element? where Element: StreamableNumeric {
        let expectedFormat = OptionalFormat(fixedSize: MemoryLayout<Element>.size)
        guard try readOptional(tag: tag, expectedFormat: expectedFormat!) else {
            return nil
        }
        return try read()
    }

    /// Reads a sequence of numeric values from the stream.
    ///
    /// - returns: `[Element]` - The sequence of numeric values read from the stream.
    public func read<Element>() throws -> [Element] where Element: StreamableNumeric {
        let sz = try readAndCheckSeqSize(minSize: MemoryLayout<Element>.size)

        if sz == 0 {
            return [Element]()
        } else {
            let eltSize = MemoryLayout<Element>.size
            if sz == 1 || eltSize == MemoryLayout<Element>.stride {
                // Can copy directly from bytes to array
                var a = [Element](repeating: 0, count: sz)
                pos += a.withUnsafeMutableBufferPointer { buf in
                    self.data.copyBytes(to: buf, from: self.pos..<self.pos + sz * eltSize)
                }
                return a
            } else {
                var a = [Element]()
                a.reserveCapacity(sz)
                for _ in 0..<sz {
                    try a.append(read())
                }
                return a
            }
        }
    }

    /// Reads an optional sequence of numeric values from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `[Element]?` - The optional sequence read from the stream.
    public func read<Element>(tag: Int32) throws -> [Element]? where Element: StreamableNumeric {
        guard try readOptional(tag: tag, expectedFormat: .VSize) else {
            return nil
        }
        if MemoryLayout<Element>.size > 1 {
            try skipSize()
        }
        return try read()
    }

    /// Reads a byte from the stream.
    ///
    /// - returns: `UInt8` - The byte read from the stream.
    public func read() throws -> UInt8 {
        guard remaining > 0 else {
            throw MarshalException(endOfBufferMessage)
        }
        let value = data[pos]
        pos += 1
        return value
    }

    /// Reads a sequence of bytes from the stream.
    ///
    /// - returns: `[UInt8]` - The sequence of bytes read from the stream.
    public func read() throws -> [UInt8] {
        let sz = try readAndCheckSeqSize(minSize: 1)
        let start = pos
        pos += sz
        return [UInt8](data[start..<pos])
    }

    /// Reads a sequence of bytes from the stream.
    ///
    /// - returns: `Data` - The sequence of bytes read from the stream.
    public func read() throws -> Data {
        let sz = try readAndCheckSeqSize(minSize: 1)
        let start = pos
        pos += sz
        return data.subdata(in: start..<pos)  // copy
    }

    /// Reads an optional sequence of bytes from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `Data?` - The optional sequence of bytes read from the stream.
    public func read(tag: Int32) throws -> Data? {
        guard try readOptional(tag: tag, expectedFormat: .VSize) else {
            return nil
        }
        // No skipSize here
        return try read()
    }

    /// Reads a boolean value from the stream.
    ///
    /// - returns: `Bool` - The boolean value read from the stream.
    public func read() throws -> Bool {
        let value: UInt8 = try read()
        return value == 1
    }

    /// Reads an optional boolean value from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `Bool?` - The optional boolean value read from the stream.
    public func read(tag: Int32) throws -> Bool? {
        guard try readOptional(tag: tag, expectedFormat: .F1) else {
            return nil
        }
        return try read() as Bool
    }

    /// Reads a sequence of boolean value from the stream.
    ///
    /// - returns: `[Bool]` - The sequence of boolean values read from the stream.
    public func read() throws -> [Bool] {
        let sz = try readAndCheckSeqSize(minSize: 1)

        if sz == 0 {
            return [Bool]()
        } else if MemoryLayout<Bool>.size == 1, MemoryLayout<Bool>.stride == 1 {
            // Copy directly from bytes to array
            var a = [Bool](repeating: false, count: sz)
            pos += a.withUnsafeMutableBufferPointer { buf in
                self.data.copyBytes(to: buf, from: self.pos..<self.pos + sz)
            }
            return a
        } else {
            fatalError("Unsupported Bool memory layout")
        }
    }

    /// Reads an optional sequence of boolean value from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `[Bool]?` - The optional sequence of boolean values read from the stream.
    public func read(tag: Int32) throws -> [Bool]? {
        guard try readOptional(tag: tag, expectedFormat: .VSize) else {
            return nil
        }
        return try read()
    }

    /// Reads a size from the stream.
    ///
    /// - returns: `Int32` - The size read from the stream.
    public func readSize() throws -> Int32 {
        let byteVal: UInt8 = try read()
        if byteVal == 255 {
            return try read()
        } else {
            return Int32(byteVal)
        }
    }

    /// Reads a sequence size from the stream and ensures the stream has enough
    /// bytes for `size` elements, where each element's size is at least minSize.
    ///
    /// - parameter minSize: `Int` - The mininum element size to use for the check.
    ///
    /// - returns: `Int` - The size read from the stream.
    public func readAndCheckSeqSize(minSize: Int) throws -> Int {
        let sz = try Int(readSize())

        guard sz != 0 else {
            return sz
        }

        //
        // The startSeq variable points to the start of the sequence for which
        // we expect to read at least minSeqSize bytes from the stream.
        //
        // If not initialized or if we already read more data than minSeqSize,
        // we reset startSeq and minSeqSize for this sequence (possibly a
        // top-level sequence or enclosed sequence it doesn't really matter).
        //
        // Otherwise, we are reading an enclosed sequence and we have to bump
        // minSeqSize by the minimum size that this sequence will require on
        // the stream.
        //
        // The goal of this check is to ensure that when we start un-marshaling
        // a new sequence, we check the minimal size of this new sequence against
        // the estimated remaining buffer size. This estimation is based on
        // the minimum size of the enclosing sequences, it's minSeqSize.
        //
        if startSeq == -1 || pos > (startSeq + minSeqSize) {
            startSeq = Int32(pos)
            minSeqSize = Int32(sz * minSize)
        } else {
            minSeqSize += Int32(sz * minSize)
        }

        //
        // If there isn't enough data to read on the stream for the sequence (and
        // possibly enclosed sequences), something is wrong with the marshaled
        // data: it's claiming having more data that what is possible to read.
        //
        if startSeq + minSeqSize > data.count {
            throw MarshalException(endOfBufferMessage)
        }

        return sz
    }

    //
    // Optional
    //
    public func readOptional(tag: Int32, expectedFormat: OptionalFormat) throws -> Bool {
        if encaps.decoder != nil {
            return try encaps.decoder.readOptional(tag: tag, format: expectedFormat)
        }

        return try readOptionalImpl(readTag: tag, expectedFormat: expectedFormat)
    }

    internal func readOptionalImpl(readTag: Int32, expectedFormat: OptionalFormat) throws -> Bool {
        if encaps.encoding_1_0 {
            return false  // Optional members aren't supported with the 1.0 encoding.
        }

        while true {
            if pos >= encaps.start + encaps.sz {
                return false  // End of encapsulation also indicates end of optionals.
            }

            let v: UInt8 = try read()
            if v == SliceFlags.OPTIONAL_END_MARKER.rawValue {
                try changePos(offset: -1)  // Rewind
                return false
            }

            // First 3 bits.
            guard let format = OptionalFormat(rawValue: v & 0x07) else {
                throw MarshalException("invalid optional format")
            }
            var tag = Int32(v >> 3)
            if tag == 30 {
                tag = try readSize()
            }

            if tag > readTag {
                let offset = tag < 30 ? -1 : (tag < 255 ? -2 : -6)  // Rewind
                try changePos(offset: offset)
                return false  // No optional data members with the requested tag
            } else if tag < readTag {
                try skipOptional(format: format)  // Skip optional data members
            } else {
                if format != expectedFormat {
                    throw MarshalException(
                        "invalid optional data member `\(tag)': unexpected format")
                }
                return true
            }
        }
    }

    /// Reads an enumerator from the stream, as a byte.
    ///
    /// - parameter enumMaxValue: `Int32` - The maximum value for the enumerators (used only for the 1.0 encoding).
    ///
    /// - returns: `UInt8` - The enumerator's byte value.
    public func read(enumMaxValue: Int32) throws -> UInt8 {
        if currentEncoding == Encoding_1_0 {
            if enumMaxValue < 127 {
                return try read()
            } else if enumMaxValue < 32767 {
                let v: Int16 = try read()
                guard v <= UInt8.max else {
                    throw MarshalException("unexpected enumerator value")
                }
                return UInt8(v)
            } else {
                let v: Int32 = try read()
                guard v <= UInt8.max else {
                    throw MarshalException("unexpected enumerator value")
                }
                return UInt8(v)
            }
        } else {
            let v = try readSize()
            guard v <= UInt8.max else {
                throw MarshalException("unexpected enumerator value")
            }
            return UInt8(v)
        }
    }

    /// Reads an enumerator from the stream, as a Int32.
    ///
    /// - parameter enumMaxValue: `Int32` - The maximum value for the enumerators (used only for the 1.0 encoding).
    ///
    /// - returns: `Int32` - The enumerator's Int32 value.
    public func read(enumMaxValue: Int32) throws -> Int32 {
        if currentEncoding == Encoding_1_0 {
            if enumMaxValue < 127 {
                return try Int32(read() as UInt8)
            } else if enumMaxValue < 32767 {
                return try Int32(read() as Int16)
            } else {
                return try read()
            }
        } else {
            return try readSize()
        }
    }

    /// Reads a string from the stream.
    ///
    /// - returns: `String` - The string read from the stream.
    public func read() throws -> String {
        let size = try readSize()
        if size == 0 {
            return ""
        } else {
            let start = pos
            try skip(size)
            let end = pos
            guard let str = String(data: data[start..<end], encoding: .utf8) else {
                throw MarshalException("unable to read string")
            }
            return str
        }
    }

    /// Reads an optional string from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `String?` - The optional string read from the stream.
    public func read(tag: Int32) throws -> String? {
        guard try readOptional(tag: tag, expectedFormat: .VSize) else {
            return nil
        }
        return try read() as String
    }

    /// Reads a sequence of strings from the stream.
    ///
    /// - returns: `[String]` - The sequence of strings read from the stream.
    public func read() throws -> [String] {
        let sz = try readAndCheckSeqSize(minSize: 1)
        var r = [String]()
        r.reserveCapacity(sz)
        for _ in 0..<sz {
            try r.append(read())
        }
        return r
    }

    /// Reads an optional sequence of strings from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `[String]?` - The optional sequence of strings read from the stream.
    public func read(tag: Int32) throws -> [String]? {
        guard try readOptional(tag: tag, expectedFormat: .FSize) else {
            return nil
        }
        try skip(4)
        return try read() as [String]
    }

    /// Reads a proxy from the stream (internal helper).
    ///
    /// - returns: `ProxyImpl?` - The proxy read from the stream.
    public func read<ProxyImpl>() throws -> sending ProxyImpl? where ProxyImpl: ObjectPrxI {
        return try ProxyImpl.ice_read(from: self)
    }

    /// Reads an optional proxy from the stream (internal helper).
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - returns: `ProxyImpl?` - The proxy read from the stream.
    public func read<ProxyImpl>(tag: Int32) throws -> sending ProxyImpl?
    where ProxyImpl: ObjectPrxI {
        guard try readOptional(tag: tag, expectedFormat: .FSize) else {
            return nil
        }
        try skip(4)
        return try read() as ProxyImpl?
    }

    /// Reads a base proxy from the stream.
    ///
    /// - returns: `ObjectPrx?` - The proxy read from the stream.
    public func read(_: ObjectPrx.Protocol) throws -> sending ObjectPrx? {
        return try read() as ObjectPrxI?
    }

    /// Reads an optional base proxy from the stream.
    ///
    /// - parameter tag: `Int32` - The tag of the optional data member or parameter.
    ///
    /// - parameter type: `ObjectPrx.Protocol` - The proxy type.
    ///
    /// - returns: `ObjectPrx?` - The proxy read from the stream.
    public func read(tag: Int32, type _: ObjectPrx.Protocol) throws -> sending ObjectPrx? {
        return try read(tag: tag) as ObjectPrxI?
    }

    /// Reads a value from the stream.
    // - Parameter cb: The closure that sets the value in the caller.
    //                 This closure always executed in the same thread as the caller.
    // TODO: We marked cb as @Sendable to work-around Swift 6 concurrency check errors.
    public func read(cb: @Sendable @escaping (Value?) throws -> Void) throws {
        initEncaps()
        try encaps.decoder.readValue(cb: cb)
    }

    /// Reads a value from the stream.
    // - Parameter cb: The closure that sets the value in the caller.
    //                 This closure always executed in the same thread as the caller.
    // TODO: We marked cb as @Sendable to work-around Swift 6 concurrency check errors.
    public func read<ValueType>(
        _ value: ValueType.Type, cb: @Sendable @escaping (ValueType?) -> Void
    ) throws
    where ValueType: Value {
        initEncaps()
        try encaps.decoder.readValue { v in
            if v == nil || v is ValueType {
                cb(v as? ValueType)
            } else {
                try InputStream.throwUOE(expectedType: value, v: v!)
            }
        }
    }
}

private class Encaps {
    let start: Int
    let sz: Int
    let encoding: EncodingVersion
    let encoding_1_0: Bool
    var decoder: EncapsDecoder!

    init(start: Int, size: Int, encoding: EncodingVersion) {
        self.start = start
        sz = size
        self.encoding = encoding
        encoding_1_0 = encoding == Encoding_1_0
    }
}

private enum SliceType {
    case NoSlice
    case ValueSlice
    case ExceptionSlice
}

private typealias Callback = (Value?) throws -> Void

private struct PatchEntry {
    let cb: Callback
    let classGraphDepth: Int32

    fileprivate init(cb: @escaping Callback, classGraphDepth: Int32) {
        self.cb = cb
        self.classGraphDepth = classGraphDepth
    }
}

private protocol EncapsDecoder: AnyObject {
    var stream: InputStream { get }

    //
    // Encapsulation attributes for value unmarshaling.
    //
    var patchMap: [Int32: [PatchEntry]] { get set }
    var unmarshaledMap: [Int32: Value?] { get set }
    var typeIdMap: [Int32: String] { get set }
    var typeIdIndex: Int32 { get set }
    var valueList: [Value] { get set }

    var classGraphDepthMax: Int32 { get }
    var classGraphDepth: Int32 { get set }

    func readValue(cb: @escaping Callback) throws
    func throwException() throws

    func startInstance(type: SliceType)
    func endInstance() throws -> SlicedData?
    func startSlice() throws
    func endSlice() throws
    func skipSlice() throws

    func readOptional(tag: Int32, format: OptionalFormat) throws -> Bool
    func readPendingValues() throws
}

extension EncapsDecoder {
    func readOptional(tag _: Int32, format _: OptionalFormat) throws -> Bool {
        return false
    }

    func readPendingValues() throws {}

    func readTypeId(isIndex: Bool) throws -> String {
        if isIndex {
            let index = try stream.readSize()
            guard let typeId = typeIdMap[index] else {
                throw MarshalException("invalid typeId")
            }
            return typeId
        } else {
            let typeId: String = try stream.read()
            typeIdIndex += 1
            typeIdMap[typeIdIndex] = typeId
            return typeId
        }
    }

    func newInstance(typeId: String) throws -> Value? {
        stream.sliceLoader.newInstance(typeId) as? Value
    }

    func addPatchEntry(index: Int32, cb: @escaping Callback) throws {
        precondition(index > 0, "invalid index")

        //
        // Check if we already unmarshaled the object. If that's the case, just patch the object smart pointer
        // and we're done. A null value indicates we've encountered a cycle and Ice.AllowClassCycles is false.
        //
        if let optObj = unmarshaledMap[index] {
            guard let obj = optObj else {
                assert(!stream.acceptClassCycles)
                throw MarshalException("cycle detected during Value unmarshaling")
            }
            try cb(obj)
            return
        }

        //
        // Add patch entry if the instance isn't unmarshaled yet,
        // the callback will be called when the instance is
        // unmarshaled.
        //
        var entries = patchMap[index] ?? []
        entries.append(PatchEntry(cb: cb, classGraphDepth: classGraphDepth))
        patchMap[index] = entries
    }

    func unmarshal(index: Int32, v: Value) throws {
        //
        // Add the instance to the map of unmarshaled instances, this must
        // be done before reading the instances (for circular references).
        //
        // If circular references are not allowed we insert null (for cycle detection) and add
        // the object to the map once it has been fully unmarshaled.
        //
        unmarshaledMap[index] = stream.acceptClassCycles ? v : (nil as Value?)
        assert(unmarshaledMap[index] != nil)

        //
        // Read the instance.
        //
        try v._iceRead(from: stream)

        //
        // Patch all instances now that the instance is unmarshaled.
        //
        if let l = patchMap[index] {
            precondition(!l.isEmpty)

            //
            // Patch all pointers that refer to the instance.
            //
            for entry in l {
                try entry.cb(v)
            }

            //
            // Clear out the patch map for that index -- there is nothing left
            // to patch for that index for the time being.
            //
            patchMap.removeValue(forKey: index)
        }

        if patchMap.isEmpty, valueList.isEmpty {
            v.ice_postUnmarshal()
        } else {
            valueList.append(v)

            if patchMap.isEmpty {
                //
                // Iterate over the instance list and invoke ice_postUnmarshal on
                // each instance. We must do this after all instances have been
                // unmarshaled in order to ensure that any instance data members
                // have been properly patched.
                //
                for p in valueList {
                    p.ice_postUnmarshal()
                }
                valueList.removeAll()
            }
        }

        if !stream.acceptClassCycles {
            // This class has been fully unmarshaled without creating any cycles
            // It can be added to the map now.
            unmarshaledMap[index] = v
        }
    }
}

private class EncapsDecoder10: EncapsDecoder {
    // EncapsDecoder members
    unowned let stream: InputStream
    lazy var patchMap = [Int32: [PatchEntry]]()
    lazy var unmarshaledMap = [Int32: Value?]()
    lazy var typeIdMap = [Int32: String]()
    var typeIdIndex: Int32 = 0
    lazy var valueList = [Value]()

    // Value/exception attributes
    var sliceType: SliceType
    var skipFirstSlice: Bool!

    // Slice attributes
    var sliceSize: Int32!
    var typeId: String!

    let classGraphDepthMax: Int32
    var classGraphDepth: Int32

    init(stream: InputStream, classGraphDepthMax: Int32) {
        self.stream = stream
        sliceType = SliceType.NoSlice
        self.classGraphDepthMax = classGraphDepthMax
        classGraphDepth = 0
    }

    func readValue(cb: @escaping Callback) throws {
        //
        // Object references are encoded as a negative integer in 1.0.
        //
        var index: Int32 = try stream.read()
        if index > 0 {
            throw MarshalException("invalid object id")
        }
        index = -index

        if index == 0 {
            try cb(nil)
        } else {
            try addPatchEntry(index: index, cb: cb)
        }
    }

    func throwException() throws {
        precondition(sliceType == .NoSlice)

        //
        // User exception with the 1.0 encoding start with a boolean flag
        // that indicates whether or not the exception has classes.
        //
        // This allows reading the pending instances even if some part of
        // the exception was sliced.
        //

        let usesClasses: Bool = try stream.read()

        sliceType = .ExceptionSlice
        skipFirstSlice = false

        //
        // Read the first slice header.
        //
        try startSlice()
        let mostDerivedId = typeId!

        while true {
            if let obj = stream.sliceLoader.newInstance(typeId) {
                let ex = obj as! UserException
                try ex._iceRead(from: stream)
                if usesClasses {
                    try readPendingValues()
                }
                throw ex
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()
            do {
                try startSlice()
            } catch is MarshalException {
                //
                // An oversight in the 1.0 encoding means there is no marker to indicate
                // the last slice of an exception. As a result, we just try to read the
                // next type ID, which raises MarshalException when the
                // input buffer underflows.

                throw MarshalException("unknown exception type '\(mostDerivedId)'")
            }
        }
    }

    func startInstance(type: SliceType) {
        precondition(sliceType == type)
        skipFirstSlice = true
    }

    func endInstance() throws -> SlicedData? {
        //
        // Read the Ice::Value slice.
        //
        if sliceType == .ValueSlice {
            try startSlice()
            let sz = try stream.readSize()  // For compatibility with the old AFM.
            if sz != 0 {
                throw MarshalException("invalid Object slice")
            }
            try endSlice()
        }

        sliceType = .NoSlice
        return nil
    }

    func startSlice() throws {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if skipFirstSlice {
            skipFirstSlice = false
            return
        }

        //
        // For class instances, first read the type ID boolean which indicates
        // whether or not the type ID is encoded as a string or as an
        // index. For exceptions, the type ID is always encoded as a
        // string.
        //
        if sliceType == .ValueSlice {
            let isIndex: Bool = try stream.read()
            typeId = try readTypeId(isIndex: isIndex)
        } else {
            typeId = try stream.read()
        }

        sliceSize = try stream.read()
        if sliceSize < 4 {
            throw MarshalException("unexpected slice size")
        }
    }

    func endSlice() throws {}

    func skipSlice() throws {
        stream.traceSkipSlice(typeId: typeId, sliceType: sliceType)
        try stream.skip(sliceSize - 4)
    }

    func readPendingValues() throws {
        var num: Int32
        repeat {
            num = try stream.readSize()
            for _ in 0..<num {
                try readInstance()
            }
        } while num > 0

        if !patchMap.isEmpty {
            //
            // If any entries remain in the patch map, the sender has sent an index for an object, but failed
            // to supply the object.
            //
            throw MarshalException("index for class received, but no instance")
        }
    }

    func readInstance() throws {
        let index: Int32 = try stream.read()

        if index <= 0 {
            throw MarshalException("invalid object id")
        }

        sliceType = SliceType.ValueSlice
        skipFirstSlice = false

        //
        // Read the first slice header.
        //
        try startSlice()
        let mostDerivedId = typeId!
        var v: Value!

        while true {
            // For the 1.0 encoding, the type ID for the base Object class marks the last slice.
            if typeId == "::Ice::Object" {
                throw MarshalException(
                    "The Slice loader did not find a class for type ID '\(mostDerivedId)'")
            }

            v = try newInstance(typeId: typeId)

            //
            // We found a factory, we get out of this loop.
            //
            if v != nil {
                break
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()
            try startSlice()  // Read next Slice header for next iteration.
        }

        //
        // Compute the biggest class graph depth of this object. To compute this,
        // we get the class graph depth of each ancestor from the patch map and
        // keep the biggest one.
        //
        classGraphDepth = 0
        if let l = patchMap[index] {
            precondition(l.count > 0)
            classGraphDepth = l.reduce(0) { max($0, $1.classGraphDepth) }
        }
        classGraphDepth += 1
        if classGraphDepth > classGraphDepthMax {
            throw MarshalException("maximum class graph depth reached")
        }

        //
        // Unmarshal the instance and add it to the map of unmarshaled instances.
        //
        try unmarshal(index: index, v: v)
    }
}

private class EncapsDecoder11: EncapsDecoder {
    // EncapsDecoder members
    unowned let stream: InputStream
    lazy var patchMap = [Int32: [PatchEntry]]()
    lazy var unmarshaledMap = [Int32: Value?]()
    lazy var typeIdMap = [Int32: String]()
    var typeIdIndex: Int32 = 0
    lazy var valueList = [Value]()

    let classGraphDepthMax: Int32
    var classGraphDepth: Int32

    private var current: InstanceData!
    var valueIdIndex: Int32 = 1  // The ID of the next instance to unmarshal.

    private struct IndirectPatchEntry {
        var index: Int32
        var cb: Callback

        init(index: Int32, cb: @escaping Callback) {
            self.index = index
            self.cb = cb
        }
    }

    private class InstanceData {
        // Instance attributes
        var sliceType: SliceType!
        var skipFirstSlice: Bool!
        lazy var slices = [SliceInfo]()  // Preserved slices.
        lazy var indirectionTables = [[Int32]]()

        // Slice attributes
        var sliceFlags: SliceFlags!
        var sliceSize: Int32!
        var typeId: String!
        var compactId: Int32!
        lazy var indirectPatchList = [IndirectPatchEntry]()

        let previous: InstanceData?
        weak var next: InstanceData?

        init(previous: InstanceData?) {
            self.previous = previous
            next = nil

            previous?.next = self
        }
    }

    init(stream: InputStream, classGraphDepthMax: Int32) {
        self.stream = stream
        self.classGraphDepthMax = classGraphDepthMax
        classGraphDepth = 0
    }

    func readValue(cb: @escaping Callback) throws {
        let index = try stream.readSize()
        if index < 0 {
            throw MarshalException("invalid object id")
        } else if index == 0 {
            try cb(nil)
        } else if current != nil, current.sliceFlags.contains(.FLAG_HAS_INDIRECTION_TABLE) {
            //
            // When reading a class instance within a slice and there's an
            // indirect instance table, always read an indirect reference
            // that points to an instance from the indirect instance table
            // marshaled at the end of the Slice.
            //
            // Maintain a list of indirect references. Note that the
            // indirect index starts at 1, so we decrement it by one to
            // derive an index into the indirection table that we'll read
            // at the end of the slice.
            //
            current.indirectPatchList.append(IndirectPatchEntry(index: index - 1, cb: cb))
        } else {
            _ = try readInstance(index: index, cb: cb)
        }
    }

    func throwException() throws {
        precondition(current == nil)

        push(sliceType: .ExceptionSlice)
        //
        // Read the first slice header.
        //
        try startSlice()
        let mostDerivedId = current.typeId!
        while true {
            if let obj = stream.sliceLoader.newInstance(current.typeId) {
                let ex = obj as! UserException
                try ex._iceRead(from: stream)
                throw ex
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()

            if current.sliceFlags.contains(.FLAG_IS_LAST_SLICE) {
                throw MarshalException(
                    "cannot unmarshal user exception with type ID '\(mostDerivedId)'")
            }

            try startSlice()
        }
    }

    func startInstance(type: SliceType) {
        precondition(current.sliceType == type)
        current.skipFirstSlice = true
    }

    func endInstance() throws -> SlicedData? {
        let slicedData = try readSlicedData()

        current.slices.removeAll()
        current.indirectionTables.removeAll()

        current = current.previous
        return slicedData
    }

    func startSlice() throws {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if current.skipFirstSlice {
            current.skipFirstSlice = false
            return
        }

        current.sliceFlags = try SliceFlags(rawValue: stream.read())

        //
        // Read the type ID, for value slices the type ID is encoded as a
        // string or as an index, for exceptions it's always encoded as a
        // string.
        //
        if current.sliceType == .ValueSlice {
            // Must be checked 1st!
            if current.sliceFlags.contains(.FLAG_HAS_TYPE_ID_COMPACT) {
                current.compactId = try stream.readSize()
                current.typeId = "\(current.compactId!)"
            } else if current.sliceFlags.contains(.FLAG_HAS_TYPE_ID_INDEX)
                || current.sliceFlags.contains(.FLAG_HAS_TYPE_ID_STRING)
            {
                current.typeId = try readTypeId(
                    isIndex: current.sliceFlags.contains(.FLAG_HAS_TYPE_ID_INDEX))
                current.compactId = -1
            } else {
                // Only the most derived slice encodes the type ID for the compact format.
                current.typeId = ""
                current.compactId = -1
            }
        } else {
            current.typeId = try stream.read()
            current.compactId = -1
        }

        //
        // Read the slice size if necessary.
        //
        if current.sliceFlags.contains(SliceFlags.FLAG_HAS_SLICE_SIZE) {
            current.sliceSize = try stream.read()
            if current.sliceSize < 4 {
                throw MarshalException("invalid slice size")
            }
        } else {
            current.sliceSize = 0
        }
    }

    func endSlice() throws {
        if current.sliceFlags.contains(.FLAG_HAS_OPTIONAL_MEMBERS) {
            try stream.skipOptionals()
        }

        //
        // Read the indirection table if one is present and transform the
        // indirect patch list into patch entries with direct references.
        //
        if current.sliceFlags.contains(.FLAG_HAS_INDIRECTION_TABLE) {
            var indirectionTable = try [Int32](
                repeating: 0, count: Int(stream.readAndCheckSeqSize(minSize: 1)))

            for i in 0..<indirectionTable.count {
                indirectionTable[i] = try readInstance(index: stream.readSize(), cb: nil)
            }

            //
            // Sanity checks. If there are optional members, it's possible
            // that not all instance references were read if they are from
            // unknown optional data members.
            //
            if indirectionTable.isEmpty {
                throw MarshalException("empty indirection table")
            }
            if current.indirectPatchList.isEmpty,
                !current.sliceFlags.contains(.FLAG_HAS_OPTIONAL_MEMBERS)
            {
                throw MarshalException("no references to indirection table")
            }

            //
            // Convert indirect references into direct references.
            //
            for e in current.indirectPatchList {
                precondition(e.index >= 0)
                if e.index >= indirectionTable.count {
                    throw MarshalException("indirection out of range")
                }
                try addPatchEntry(index: indirectionTable[Int(e.index)], cb: e.cb)
            }
            current.indirectPatchList.removeAll()
        }
    }

    func skipSlice() throws {
        stream.traceSkipSlice(typeId: current.typeId, sliceType: current.sliceType)

        let start = stream.pos

        if current.sliceFlags.contains(.FLAG_HAS_SLICE_SIZE) {
            precondition(current.sliceSize >= 4)
            try stream.skip(current.sliceSize - 4)
        } else {
            if current.sliceType == .ValueSlice {
                throw MarshalException(
                    "The Slice loader did not find a class for type ID '\(current.typeId!)' and compact format prevents slicing."
                )
            } else {
                throw MarshalException(
                    "The Slice loader did not find a user exception class for type ID '\(current.typeId!)' and compact format prevents slicing."
                )
            }
        }

        //
        // Preserve this slice if unmarshaling a value in Slice format. Exception slices are not preserved.
        //
        if current.sliceType == .ValueSlice {
            let hasOptionalMembers = current.sliceFlags.contains(.FLAG_HAS_OPTIONAL_MEMBERS)
            let isLastSlice = current.sliceFlags.contains(.FLAG_IS_LAST_SLICE)
            var dataEnd = stream.pos

            if hasOptionalMembers {
                //
                // Don't include the optional member end marker. It will be re-written by
                // endSlice when the sliced data is re-written.
                //
                dataEnd -= 1
            }

            let bytes = stream.data.subdata(in: start..<dataEnd)  // copy

            let info = SliceInfo(
                typeId: current.typeId,
                compactId: current.compactId,
                bytes: bytes,
                instances: [],
                hasOptionalMembers: hasOptionalMembers,
                isLastSlice: isLastSlice)

            current.slices.append(info)
        }

        //
        // Read the indirect instance table. We read the instances or their
        // IDs if the instance is a reference to an already unmarhshaled
        // instance.
        //
        if current.sliceFlags.contains(.FLAG_HAS_INDIRECTION_TABLE) {
            var indirectionTable = try [Int32](
                repeating: 0, count: Int(stream.readAndCheckSeqSize(minSize: 1)))

            for i in 0..<indirectionTable.count {
                indirectionTable[i] = try readInstance(index: stream.readSize(), cb: nil)
            }
            current.indirectionTables.append(indirectionTable)
        } else {
            current.indirectionTables.append([])
        }
    }

    func readOptional(tag: Int32, format: OptionalFormat) throws -> Bool {
        if current == nil {
            return try stream.readOptionalImpl(readTag: tag, expectedFormat: format)
        } else if current.sliceFlags.contains(.FLAG_HAS_OPTIONAL_MEMBERS) {
            return try stream.readOptionalImpl(readTag: tag, expectedFormat: format)
        }
        return false
    }

    func readInstance(index: Int32, cb: Callback?) throws -> Int32 {
        precondition(index > 0)

        if index > 1 {
            if let cb = cb {
                try addPatchEntry(index: index, cb: cb)
            }
            return index
        }

        push(sliceType: .ValueSlice)

        //
        // Get the instance ID before we start reading slices. If some
        // slices are skipped, the indirect instance table is still read and
        // might read other instances.
        //
        valueIdIndex += 1
        let index = valueIdIndex

        //
        // Read the first slice header.
        //
        try startSlice()
        let mostDerivedId = current.typeId!

        var v: Value? = nil
        while true {
            if !current.typeId.isEmpty {
                v = try newInstance(typeId: current.typeId)
                if v != nil {
                    break
                }
            }

            // Slice off what we don't understand.
            try skipSlice()

            // If this is the last slice, keep the instance as an opaque UnknownSlicedValue object.
            if current.sliceFlags.contains(.FLAG_IS_LAST_SLICE) {
                // Provide the Slice loader with an opportunity to supply the instance.
                // We pass the "::Ice::Object" ID to indicate that this is the last chance to preserve the instance.
                v = try newInstance(typeId: "::Ice::Object")
                if v == nil {
                    v = UnknownSlicedValue(unknownTypeId: mostDerivedId)
                }

                break
            }

            try startSlice()  // Read next Slice header for next iteration.
        }

        classGraphDepth += 1
        if classGraphDepth > classGraphDepthMax {
            throw MarshalException("maximum class graph depth reached")
        }

        //
        // Unmarshal the instance.
        //
        try unmarshal(index: index, v: v!)

        classGraphDepth -= 1

        if current == nil, !patchMap.isEmpty {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw MarshalException("index for class received, but no instance")
        }

        try cb?(v)

        return index
    }

    func readSlicedData() throws -> SlicedData? {
        // No preserved slices.
        if current.slices.isEmpty {
            return nil
        }

        //
        // The _indirectionTables member holds the indirection table for each slice
        // in _slices.
        //
        precondition(current.slices.count == current.indirectionTables.count)

        for n in 0..<current.slices.count {
            //
            // We use the "instances" list in SliceInfo to hold references
            // to the target instances. Note that the instances might not have
            // been read yet in the case of a circular reference to an
            // enclosing instance.

            let sz = current.indirectionTables[n].count
            current.slices[n].instances = [Ice.Value]()
            current.slices[n].instances.reserveCapacity(sz)
            for j in 0..<sz {
                try addPatchEntry(index: current.indirectionTables[n][j]) { v in
                    self.current.slices[n].instances.append(v!)
                }
            }
        }

        return SlicedData(slices: current.slices)
    }

    func push(sliceType: SliceType) {
        if current == nil {
            current = InstanceData(previous: nil)
        } else {
            current = current.next ?? InstanceData(previous: current)
        }
        current.sliceType = sliceType
        current.skipFirstSlice = false
    }
}

public struct DictEntry<K, V> {
    public var key: K!
    public var value: UnsafeMutablePointer<V>!

    public init(key: K? = nil, value: UnsafeMutablePointer<V>? = nil) {
        self.key = key
        self.value = value
    }
}

public class DictEntryArray<K, V> {
    public var values: [DictEntry<K, V>]

    public init(size: Int) {
        values = [DictEntry<K, V>](repeating: DictEntry<K, V>(), count: size)
    }
}

/// A Numeric type that can be marshaled (written) using an OutputStream and
/// unmarshaled (read) using an InputStream
public protocol StreamableNumeric: Numeric {}

extension UInt8: StreamableNumeric {}
extension Int16: StreamableNumeric {}
extension Int32: StreamableNumeric {}
extension Int64: StreamableNumeric {}
extension Float: StreamableNumeric {}
extension Double: StreamableNumeric {}
