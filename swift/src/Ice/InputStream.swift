// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

public class InputStream {
    // An input stream can be represented by either:
    // - A handle to the bytes from an ObjC (C++) InputStream (unowned)
    // - A byte array passed and copied during initialization (owned)
    private var handle: ICEInputStream!
    private var bytes: [UInt8]!

    private var buf: Buffer // buf overlays the C++ unmarshal buffer stored in ICEInputStream or user buffer
    private var communicator: Communicator
    private var encoding: EncodingVersion
    private var traceSlicing: Bool

    private var encaps: Encaps!

    private var startSeq: Int32 = -1
    private var minSeqSize: Int32 = 0
    public var sliceValues: Bool = true

    public convenience init(communicator: Communicator) {
        self.init(communicator: communicator, bytes: [])
    }

    public convenience init(communicator: Communicator, bytes: [UInt8]) {
        let encoding = (communicator as! CommunicatorI).defaultsAndOverrides.defaultEncoding
        self.init(communicator: communicator, encoding: encoding, bytes: bytes)
    }

    public required init(communicator: Communicator,
                         encoding: EncodingVersion,
                         bytes: [UInt8]) {
        self.communicator = communicator
        self.encoding = encoding
        self.bytes = bytes
        var baseAddress: UnsafeMutableRawPointer?
        self.bytes.withUnsafeMutableBytes { baseAddress = $0.baseAddress }
        buf = Buffer(start: baseAddress!, count: bytes.count)
        traceSlicing = communicator.getProperties().getPropertyAsIntWithDefault(key: "Ice.Trace.Slicing", value: 0) > 0
    }

    init(communicator: Communicator, inputStream handle: ICEInputStream) {
        self.communicator = communicator
        self.encoding = currentEncoding // TODO should we get encoding from ICEInputStream?
        self.handle = handle
        buf = Buffer(start: handle.data(), count: handle.size())
        traceSlicing = communicator.getProperties().getPropertyAsIntWithDefault(key: "Ice.Trace.Slicing", value: 0) > 0
    }

    internal func getBuffer() -> Buffer {
        return buf
    }

    func getBytes() -> UnsafeMutableRawPointer? {
        return buf.baseAddress
    }

    func getConstBytes() -> UnsafeRawPointer? {
        return buf.constBaseAddress
    }

    func getSize() -> Int {
        return buf.position()
    }

    func getEncoding() -> EncodingVersion {
        return encoding
    }

    func getCommunicator() -> Communicator {
        return communicator
    }

    public func readEncapsulation() throws -> (bytes: [UInt8], encoding: EncodingVersion) {
        let sz = try readSize()
        if sz < 6 {
            throw UnmarshalOutOfBoundsException(reason: "Invalid size")
        }

        if sz - 4 > buf.capacity {
            throw UnmarshalOutOfBoundsException(reason: "Invalid size")
        }

        let _: EncodingVersion = try read()
        try self.buf.position(self.buf.position() - 6)

        let start = self.buf.baseAddress?.bindMemory(to: UInt8.self, capacity: Int(sz))
        let bytes = [UInt8](UnsafeBufferPointer(start: start, count: Int(sz)))
        return (bytes, encoding)
    }

    public func startEncapsulation() throws {
        encaps = Encaps()
        encaps.start = buf.position()
        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        let sz: Int32 = try read()

        if sz < 6 {
            throw UnmarshalOutOfBoundsException(reason: "invalid size")
        }
        if sz - 4 > buf.capacity - buf.position() {
            throw UnmarshalOutOfBoundsException(reason: "invalid size")
        }

        encaps.sz = Int(sz)

        let encoding: EncodingVersion = try read()

        try Protocol.checkSupportedEncoding(encoding)
        encaps.setEncoding(encoding)
    }

    public func endEncapsulation() throws {
        if !encaps.encoding_1_0 {
            try skipOptionals()
            if buf.position() != encaps.start + encaps.sz {
                throw EncapsulationException(reason: "buffer size does not match decoded encapsulation size")
            }
        } else if buf.position() != encaps.start + encaps.sz {
            if buf.position() + 1 != encaps.start + encaps.sz {
                throw EncapsulationException(reason: "buffer size does not match decoded encapsulation size")
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

    func skipEmptyEncapsulation() throws {
        let sz: Int32 = try read()

        if sz < 6 {
            throw EncapsulationException(reason: "invalid size")
        }

        if sz - 4 > buf.capacity - buf.position() {
            throw UnmarshalOutOfBoundsException(reason: "")
        }

        let encoding: EncodingVersion = try read()
        try Protocol.checkSupportedEncoding(encoding) // Make sure the encoding is supported.

        if encoding == Protocol.Encoding_1_0 {
            if sz != 6 {
                throw EncapsulationException(reason: "")
            }
        } else {
            //
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            //
            try buf.skip(sz - 6)
        }
    }

    func skipEncapsulation() throws {
        let sz: Int32 = try read()

        if sz < 6 {
            throw EncapsulationException(reason: "invalid size")
        }

        _ = try read() as EncodingVersion

        try buf.position(buf.position() + Int(sz) - 6)
    }

    public func startSlice() throws -> String {
        precondition(encaps.decoder != nil)
        return try encaps.decoder.startSlice()
    }

    public func endSlice() throws {
        precondition(encaps.decoder != nil)
        try encaps.decoder.endSlice()
    }

    public func skipSlice() throws {
        precondition(encaps.decoder != nil)
        try encaps.decoder.skipSlice()
    }

    /**
     * Indicates that unmarshaling is complete, except for any class instances. The application must call this method
     * only if the stream actually contains class instances. Calling <code>readPendingValues</code> triggers the
     * calls to consumers provided with {@link #readValue} to inform the application that unmarshaling of an instance
     * is complete.
     **/
    public func readPendingValues() throws {
        if encaps.decoder != nil {
            try encaps.decoder.readPendingValues()
        } else if encoding == Protocol.Encoding_1_0{
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
            try skip(read() as Int32)
        case .Class:
            try read(UnknownSlicedValue.self, cb: nil)
        }
    }

    func skipOptionals() throws {
        //
        // Skip remaining un-read optional members.
        //
        while true {
            if buf.position() >= encaps.start + encaps.sz {
                return // End of encapsulation also indicates end of optionals.
            }

            let v: UInt8 = try read()
            if v == Protocol.OPTIONAL_END_MARKER.rawValue {
                return
            }

            // Read first 3 bits.
            guard let format = OptionalFormat(rawValue: v & 0x07) else {
                preconditionFailure("invalid optional format")
            }

            if v >> 3 == 30 {
                try skipSlice()
            }

            try skipOptional(format: format)
        }
    }

    public func skip(_ count: Int32) throws {
        try buf.skip(count)
    }

    public func skipSize() throws {
        let b: UInt8 = try read()
        if b == 255 {
            try buf.skip(4)
        }
    }

    public func startValue() {
        precondition(encaps.decoder != nil)
        encaps.decoder.startInstance(type: .ValueSlice)
    }

    public func endValue(preserve: Bool) throws -> SlicedData? {
        precondition(encaps.decoder != nil)
        return try encaps.decoder.endInstance(preserve: preserve)
    }

    public func startException() {
        precondition(encaps.decoder != nil)
        encaps.decoder.startInstance(type: .ExceptionSlice)
    }

    public func endException(preserve: Bool) throws -> SlicedData? {
        precondition(encaps.decoder != nil)
        return try encaps.decoder.endInstance(preserve: preserve)
    }

    func initEncaps() {
        if encaps == nil {
            encaps = Encaps()
            encaps.setEncoding(self.encoding)
            encaps.sz = buf.capacity
        }
        if encaps.decoder == nil { // Lazy initialization
            let valueFactoryManager = communicator.getValueFactoryManager()
            if encaps.encoding_1_0 {
                encaps.decoder = EncapsDecoder10(stream: self, sliceValues: sliceValues,
                                                 valueFactoryManager: valueFactoryManager)
            } else {
                encaps.decoder = EncapsDecoder11(stream: self, sliceValues: sliceValues,
                                                 valueFactoryManager: valueFactoryManager)
            }
        }
    }

    fileprivate func traceSkipSlice(typeId: String, sliceType: SliceType) {
        guard traceSlicing else {
            return
        }

        let logger = communicator.getLogger()
        let l: ICELoggerProtocol = logger as? LoggerI ?? LoggerWrapper(handle: logger)
        ICETraceUtil.traceSlicing(kind: sliceType == SliceType.ExceptionSlice ? "exception" : "object",
                                  typeId: typeId,
                                  slicingCat: "Slicing",
                                  logger: l)
    }

    static func throwUOE(expectedType: Value.Type, v: Value) throws {
        //
        // If the object is an unknown sliced object, we didn't find an
        // value factory, in this case raise a NoValueFactoryException
        // instead.
        //
        if let usv = v as? UnknownSlicedValue {
            throw NoValueFactoryException(reason: "", type: usv.ice_id())
        }

        throw UnexpectedObjectException(reason: "expected element of type `\(expectedType)' but received `\(v)'",
                                        type: v.ice_id(),
                                        expectedType: expectedType.ice_staticId())
    }
}

public extension InputStream {
    private func readNumeric<Element>(_ e: inout Element) throws where Element: Numeric {
        try Swift.withUnsafeBytes(of: &e) {
            try self.buf.load(bytes: $0)
        }
    }

    private func readNumeric<Element>(_ type: Element.Type) throws -> Element where Element: Numeric {
        return try buf.load(as: type)
    }

    private func readNumeric<Element>(tag: Int32,
                                      expectedType: OptionalFormat) throws -> Element? where Element: Numeric {
        guard try readOptional(tag: tag, expectedFormat: expectedType) else {
            return nil
        }
        return try readNumeric(Element.self)
    }

    private func readNumeric<Element>(minSize: Int32) throws -> [Element] where Element: Numeric {
        let sz = try readAndCheckSeqSize(minSize: 1)
        var a = [Element]()
        a.reserveCapacity(sz)
        for _ in 0 ..< sz {
            a.append(try readNumeric(Element.self))
        }
        return a
    }

    //
    // UInt8
    //
    func read() throws -> UInt8 {
        return try readNumeric(UInt8.self)
    }

    func read(tag: Int32) throws -> UInt8? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F1) else {
            return nil
        }
        return try read() as UInt8
    }

    func read() throws -> [UInt8] {
        return try readNumeric(minSize: 1)
    }

    func read(tag: Int32) throws -> [UInt8]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Bool
    //
    func read() throws -> Bool {
        return try readNumeric(UInt8.self) == 1
    }

    func read(tag: Int32) throws -> Bool? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F1) else {
            return nil
        }
        return try read() as Bool
    }

    func read() throws -> [Bool] {
        let sz = try readAndCheckSeqSize(minSize: 1)
        var a = [Bool]()
        a.reserveCapacity(sz)
        for _ in 0..<sz {
            a.append(try readNumeric(UInt8.self) == 1)
        }
        return a
    }

    func read(tag: Int32) throws -> [Bool]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Int16
    //
    func read() throws -> Int16 {
        return try readNumeric(Int16.self)
    }

    func read(tag: Int32) throws -> Int16? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F2) else {
            return nil
        }
        return try read() as Int16
    }

    func read() throws -> [Int16] {
        return try readNumeric(minSize: 2)
    }

    func read(tag: Int32) throws -> [Int16]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Int32
    //
    func read() throws -> Int32 {
        return try readNumeric(Int32.self)
    }

    func read(tag: Int32) throws -> Int32? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F4) else {
            return nil
        }
        return try read() as Int32
    }

    func read() throws -> [Int32] {
        return try readNumeric(minSize: 4)
    }

    func read(tag: Int32) throws -> [Int32]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Int64
    //
    func read() throws -> Int64 {
        return try readNumeric(Int64.self)
    }

    func read(tag: Int32) throws -> Int64? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F8) else {
            return nil
        }
        return try read() as Int64
    }

    func read() throws -> [Int64] {
        return try readNumeric(minSize: 8)
    }

    func read(tag: Int32) throws -> [Int64]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Float
    //
    func read() throws -> Float {
        return try readNumeric(Float.self)
    }

    func read(tag: Int32) throws -> Float? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F8) else {
            return nil
        }
        return try read() as Float
    }

    func read() throws -> [Float] {
        return try readNumeric(minSize: 4)
    }

    func read(tag: Int32) throws -> [Float]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    //
    // Double
    //
    func read() throws -> Double {
        return try readNumeric(Double.self)
    }

    func read(tag: Int32) throws -> Double? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.F8) else {
            return nil
        }
        return try read() as Double
    }

    func read() throws -> [Double] {
        return try readNumeric(minSize: 8)
    }

    func read(tag: Int32) throws -> [Double]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read()
    }

    func readSize() throws -> Int32 {
        let b = try readNumeric(UInt8.self)
        if b == 255 {
            return try readNumeric(Int32.self)
        } else {
            return Int32(b)
        }
    }

    func readAndCheckSeqSize(minSize: Int) throws -> Int {
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
        // minSeqSize by the minimum size that this sequence will  require on
        // the stream.
        //
        // The goal of this check is to ensure that when we start un-marshalling
        // a new sequence, we check the minimal size of this new sequence against
        // the estimated remaining buffer size. This estimatation is based on
        // the minimum size of the enclosing sequences, it's minSeqSize.
        //
        if startSeq == -1 || buf.position() > (startSeq + minSeqSize) {
            startSeq = buf.position()
            minSeqSize = Int32(sz * minSize)
        } else {
            minSeqSize += Int32(sz * minSize)
        }

        //
        // If there isn't enough data to read on the stream for the sequence (and
        // possibly enclosed sequences), something is wrong with the marshalled
        // data: it's claiming having more data that what is possible to read.
        //
        if startSeq + minSeqSize > buf.capacity {
            throw UnmarshalOutOfBoundsException(reason: "")
        }

        return sz
    }

    func readOptional(tag: Int32, expectedFormat: OptionalFormat) throws -> Bool {
        if encaps.decoder != nil {
            return try encaps.decoder.readOptional(tag: tag, format: expectedFormat)
        }

        return try readOptionalImpl(readTag: tag, expectedFormat: expectedFormat)
    }

    func readOptionalImpl(readTag: Int32, expectedFormat: OptionalFormat) throws -> Bool {
        if encoding == Protocol.Encoding_1_0 {
            return false // Optional members aren't supported with the 1.0 encoding.
        }

        while true {
            if buf.position() >= encaps.start + encaps.sz {
                return false // End of encapsulation also indicates end of optionals.
            }

            let v: UInt8 = try read()
            if v == Protocol.OPTIONAL_END_MARKER.rawValue {
                try buf.position(buf.position() - 1) // Rewind
                return false
            }

            // First 3 bits.
            guard let format = OptionalFormat(rawValue: v & 0x07) else {
                throw MarshalException(reason: "invalid optional format")
            }
            var tag = Int32(v >> 3)
            if tag == 30 {
                tag = try readSize()
            }

            if tag > readTag {
                let offset = tag < 30 ? 1 : (tag < 255 ? 2 : 6) // Rewind
                try buf.position(buf.position() - offset)
                return false // No optional data members with the requested tag
            } else if tag < readTag {
                try skipOptional(format: format) // Skip optional data members
            } else {
                if format != expectedFormat {
                    throw MarshalException(reason: "invalid optional data member `\(tag)': unexpected format")
                }
                return true
            }
        }
    }

    func read(size: inout Int32) throws {
        size = try readSize()
    }

    func read(enum val: inout UInt8, maxValue: Int32) throws {
        if encoding == Protocol.Encoding_1_0 {
            if maxValue < 127 {
                val = try read()
            } else if maxValue < 32767 {
                let v: Int16 = try read()
                guard v <= UInt8.max else {
                    throw UnmarshalOutOfBoundsException(reason: "1.0 encoded enum value is larger than UInt8")
                }
                val = UInt8(v)
            } else {
                let v: Int32 = try read()
                guard v <= UInt8.max else {
                    throw UnmarshalOutOfBoundsException(reason: "1.0 encoded enum value is larger than UInt8")
                }
                val = UInt8(v)
            }
        } else {
            let v = try readSize()
            guard v <= UInt8.max else {
                throw UnmarshalOutOfBoundsException(reason: "1.0 encoded enum value is larger than UInt8")
            }
            val = UInt8(v)
        }
    }

    func read(enum val: inout Int32, maxValue: Int32) throws {
        if encoding == Protocol.Encoding_1_0 {
            if maxValue < 127 {
                val = Int32(try read() as UInt8)
            } else if maxValue < 32767 {
                val = Int32(try read() as Int16)
            } else {
                val = Int32(try read() as Int32)
            }
        } else {
            val = try readSize()
        }
    }

    func read() throws -> String {
        let size = try readSize()
        if size == 0 {
            return ""
        } else {
            let bytes = try buf.read(count: Int(size))
            guard let str = String(data: Data(bytes: bytes.baseAddress!, count: bytes.count), encoding: .utf8) else {
                throw MarshalException(reason: "unable to read string")
            }
            return str
        }
    }

    func read(string: inout String) throws {
        string = try read()
    }

    func read(tag: Int32) throws -> String? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.VSize) else {
            return nil
        }
        return try read() as String
    }

    func read() throws -> [String] {
        let sz = try readAndCheckSeqSize(minSize: 1)
        var r: [String] = [String]()
        r.reserveCapacity(sz)
        for _ in 0 ..< sz {
            r.append(try read())
        }
        return r
    }

    func read(tag: Int32) throws -> [String]? {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.FSize) else {
            return nil
        }
        return try read() as [String]
    }

    func read<ProxyImpl>() throws -> ProxyImpl? where ProxyImpl: _ObjectPrxI {
        return try ProxyImpl.ice_read(from: self)
    }

    func read<ProxyImpl>(tag: Int32) throws -> ProxyImpl? where ProxyImpl: _ObjectPrxI {
        guard try readOptional(tag: tag, expectedFormat: OptionalFormat.FSize) else {
            return nil
        }
        try skip(4)
        return try read() as ProxyImpl?
    }

    func read(_ type: ObjectPrx.Protocol) throws -> ObjectPrx? {
        return try read() as _ObjectPrxI?
    }

    func read(tag: Int32, type: ObjectPrx.Protocol) throws -> ObjectPrx? {
        return try read(tag: tag) as _ObjectPrxI?
    }

    func read(tag: Int32) throws -> ObjectPrx? {
        return try read(tag: tag) as _ObjectPrxI?
    }

    func read(cb: ((Value?) throws -> Void)?) throws {
        initEncaps()
        try encaps.decoder.readValue(cb: cb)
    }

    func read<ValueType>(_ value: ValueType.Type, cb: ((ValueType?) -> Void)?) throws where ValueType: Value {
        initEncaps()
        if let cb = cb {
            try encaps.decoder.readValue { v in
                if v == nil || v is ValueType {
                    cb(v as? ValueType)
                } else {
                    try InputStream.throwUOE(expectedType: value, v: v!)
                }
            }
        } else {
            try encaps.decoder.readValue(cb: nil)
        }
    }
}

private class Encaps {
    var start: Int = 0
    var sz: Int = 0
    var encoding: EncodingVersion = Ice.currentEncoding
    var encoding_1_0: Bool = false

    var decoder: EncapsDecoder!

    func setEncoding(_ encoding: EncodingVersion) {
        self.encoding = encoding
        self.encoding_1_0 = encoding == Ice.Encoding_1_0
    }
}

private enum SliceType {
    case NoSlice
    case ValueSlice
    case ExceptionSlice
}

private typealias Callback = (Value?) throws -> Void

private protocol EncapsDecoder: AnyObject {
    var stream: InputStream { get }
    var sliceValues: Bool { get }
    var valueFactoryManager: ValueFactoryManager { get }

    //
    // Encapsulation attributes for value unmarshaling.
    //
    var patchMap: [Int32: [Callback]] { get set }
    var unmarshaledMap: [Int32: Value] { get set }
    var typeIdMap: [Int32: String] { get set }
    var typeIdIndex: Int32 { get set }
    var valueList: [Value] { get set }

    var typeIdCache: [String: Value.Type?] { get set }

    func readValue(cb: Callback?) throws
    func throwException() throws

    func startInstance(type: SliceType)
    func endInstance(preserve: Bool) throws -> SlicedData?
    func startSlice() throws -> String
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
                throw UnmarshalOutOfBoundsException(reason: "invalid typeId")
            }
            return typeId
        } else {
            let typeId: String = try stream.read()
            typeIdIndex += 1
            typeIdMap[typeIdIndex] = typeId
            return typeId
        }
    }

    func resolveClass(typeId: String) throws -> Value.Type? {
        if let cls = typeIdCache[typeId] {
            return cls
        } else {
            let cls = ClassResolver.resolve(typeId: typeId) as Value.Type?
            typeIdCache[typeId] = cls
            return cls
        }
    }

    func newInstance(typeId: String) throws -> Value? {
        //
        // Try to find a factory registered for the specific type.
        //
        if let factory = valueFactoryManager.find(typeId) {
            if let v = factory(typeId) {
                return v
            }
        }

        //
        // If that fails, invoke the default factory if one has been
        // registered.
        //
        if let factory = valueFactoryManager.find("") {
            if let v = factory(typeId) {
                return v
            }
        }

        //
        // Last chance: try to instantiate the class dynamically.
        //
        if let cls = try resolveClass(typeId: typeId) {
            return cls.init()
        }

        return nil
    }

    func addPatchEntry(index: Int32, cb: @escaping Callback) throws {
        precondition(index > 0, "invalid index")

        //
        // Check if we have already unmarshalled the instance. If that's the case,
        // just invoke the callback and we're done.
        //
        if let obj: Value = unmarshaledMap[index] {
            try cb(obj)
            return
        }

        //
        // Add patch entry if the instance isn't unmarshaled yet,
        // the callback will be called when the instance is
        // unmarshaled.
        //
        if var entries = patchMap[index] {
            entries.append(cb)
            patchMap[index] = entries
        } else {
            patchMap[index] = [cb]
        }
    }

    func unmarshal(index: Int32, v: Value) throws {
        //
        // Add the instance to the map of unmarshaled instances, this must
        // be done before reading the instances (for circular references).
        //
        unmarshaledMap[index] = v

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
            for cb in l {
                try cb(v)
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
    }
}

private class EncapsDecoder10: EncapsDecoder {
    // EncapsDecoder members
    var stream: InputStream
    var sliceValues: Bool
    var valueFactoryManager: ValueFactoryManager
    lazy var patchMap = [Int32: [Callback]]()
    lazy var unmarshaledMap = [Int32: Value]()
    lazy var typeIdMap = [Int32: String]()
    var typeIdIndex: Int32 = 0
    lazy var valueList = [Value]()
    lazy var typeIdCache = [String: Value.Type?]()

    // Value/exception attributes
    var sliceType: SliceType
    var skipFirstSlice: Bool!

    // Slice attributes
    var sliceSize: Int32!
    var typeId: String!

    init(stream: InputStream, sliceValues: Bool, valueFactoryManager: ValueFactoryManager) {
        self.stream = stream
        self.sliceValues = sliceValues
        self.valueFactoryManager = valueFactoryManager
        sliceType = SliceType.NoSlice
    }

    func readValue(cb: Callback?) throws {
        guard let cb = cb else {
            preconditionFailure("patch fuction can not be nil")
        }

        //
        // Object references are encoded as a negative integer in 1.0.
        //
        var index: Int32 = try stream.read()
        if index > 0 {
            throw MarshalException(reason: "invalid object id")
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
            //
            // Look for user exception
            //
            var userExceptionType: UserException.Type? = ClassResolver.resolve(typeId: typeId)

            //
            // We found the exception.
            //
            if let type = userExceptionType {
                let ex = type.init()
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
            } catch let ex as UnmarshalOutOfBoundsException {
                //
                // An oversight in the 1.0 encoding means there is no marker to indicate
                // the last slice of an exception. As a result, we just try to read the
                // next type ID, which raises UnmarshalOutOfBoundsException when the
                // input buffer underflows.
                //
                // Set the reason member to a more helpful message.
                //

                ex.reason = "unknown exception type `\(mostDerivedId)'"
                throw ex
            }
        }
    }

    func startInstance(type: SliceType) {
        precondition(sliceType == type)
        skipFirstSlice = true
    }

    func endInstance(preserve _: Bool) throws -> SlicedData? {
        //
        // Read the Ice::Value slice.
        //
        if sliceType == .ValueSlice {
            try startSlice()
            let sz = try stream.readSize() // For compatibility with the old AFM.
            if sz != 0 {
                throw MarshalException(reason: "invalid Object slice")
            }
            try endSlice()
        }

        sliceType = .NoSlice
        return nil
    }

    func startSlice() throws -> String {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if skipFirstSlice {
            skipFirstSlice = false
            return typeId
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
            throw UnmarshalOutOfBoundsException(reason: "invalid slice size")
        }
        return typeId
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
            throw MarshalException(reason: "index for class received, but no instance")
        }
    }

    func readInstance() throws {
        let index: Int32 = try stream.read()

        if index <= 0 {
            throw MarshalException(reason: "invalid object id")
        }

        sliceType = SliceType.ValueSlice
        skipFirstSlice = false

        //
        // Read the first slice header.
        //
        _ = try startSlice()
        let mostDerivedId = typeId!
        var v: Value!

        while true {
            //
            // For the 1.0 encoding, the type ID for the base Object class
            // marks the last slice.
            //
            if typeId == "::Ice::Object" {
                throw NoValueFactoryException(reason: "invalid typeId", type: mostDerivedId)
            }

            v = try newInstance(typeId: typeId)

            //
            // We found a factory, we get out of this loop.
            //
            if v != nil {
                break
            }

            //
            // If slicing is disabled, stop unmarshaling.
            //
            if !sliceValues {
                throw NoValueFactoryException(reason: "no value factory found and slicing is disabled", type: typeId)
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()
            _ = try startSlice() // Read next Slice header for next iteration.
        }

        //
        // Unmarshal the instance and add it to the map of unmarshaled instances.
        //
        try unmarshal(index: index, v: v)
    }
}

private class EncapsDecoder11: EncapsDecoder {
    // EncapsDecoder members
    var stream: InputStream
    var sliceValues: Bool
    var valueFactoryManager: ValueFactoryManager
    lazy var patchMap = [Int32: [Callback]]()
    lazy var unmarshaledMap = [Int32: Value]()
    lazy var typeIdMap = [Int32: String]()
    var typeIdIndex: Int32 = 0
    lazy var valueList = [Value]()
    lazy var typeIdCache = [String: Value.Type?]()

    private var current: InstanceData!
    var valueIdIndex: Int32 = 1 // The ID of the next instance to unmarshal.
    lazy var compactIdCache = [Int32: Value.Type]() // Cache of compact type IDs.

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
        lazy var slices = [SliceInfo]() // Preserved slices.
        lazy var indirectionTables = [[Int32]]()

        // Slice attributes
        var sliceFlags: UInt8!
        var sliceSize: Int32!
        var typeId: String!
        var compactId: Int32!
        lazy var indirectPatchList = [IndirectPatchEntry]()

        let previous: InstanceData?
        var next: InstanceData?

        init(previous: InstanceData?) {
            self.previous = previous
            next = nil

            previous?.next = self
        }
    }

    init(stream: InputStream, sliceValues: Bool, valueFactoryManager: ValueFactoryManager) {
        self.stream = stream
        self.sliceValues = sliceValues
        self.valueFactoryManager = valueFactoryManager
    }

    func readValue(cb: Callback?) throws {
        let index = try stream.readSize()
        if index < 0 {
            throw MarshalException(reason: "invalid object id")
        } else if index == 0 {
            try cb?(nil)
        } else if current != nil, (current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE.rawValue) != 0 {
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
            if let cb = cb {
                current.indirectPatchList.append(IndirectPatchEntry(index: index - 1, cb: cb))
            }
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
            //
            // Look for user exception
            //
            var userExceptionType: UserException.Type?
            userExceptionType = ClassResolver.resolve(typeId: current.typeId) as UserException.Type?

            //
            // We found the exception.
            //
            if let userEx = userExceptionType {
                let ex = userEx.init()
                try ex._iceRead(from: stream)
                throw ex
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()

            if (current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE.rawValue) != 0 {
                if let range = mostDerivedId.range(of: "::") {
                    throw UnknownUserException(unknown: String(mostDerivedId[range.upperBound...]))
                } else {
                    throw UnknownUserException(unknown: mostDerivedId)
                }
            }

            try startSlice()
        }
    }

    func startInstance(type: SliceType) {
        precondition(current.sliceType == type)
        current.skipFirstSlice = true
    }

    func endInstance(preserve: Bool) throws -> SlicedData? {
        var slicedData: SlicedData?
        if preserve {
            slicedData = try readSlicedData()
        }

        current.slices.removeAll()
        current.indirectionTables.removeAll()

        current = current.previous
        return slicedData
    }

    func startSlice() throws -> String {
        //
        // If first slice, don't read the header, it was already read in
        // readInstance or throwException to find the factory.
        //
        if current.skipFirstSlice {
            current.skipFirstSlice = false
            return current.typeId
        }

        current.sliceFlags = try stream.read()

        //
        // Read the type ID, for value slices the type ID is encoded as a
        // string or as an index, for exceptions it's always encoded as a
        // string.
        //
        if current.sliceType == .ValueSlice {
            // Must be checked 1st!
            if (current.sliceFlags & Protocol.FLAG_HAS_TYPE_ID_COMPACT.rawValue) ==
                Protocol.FLAG_HAS_TYPE_ID_COMPACT.rawValue {
                current.typeId = ""
                current.compactId = try stream.readSize()
            } else if (current.sliceFlags & (Protocol.FLAG_HAS_TYPE_ID_INDEX.rawValue |
                    Protocol.FLAG_HAS_TYPE_ID_STRING.rawValue)) != 0 {
                current.typeId = try readTypeId(isIndex: (current.sliceFlags &
                        Protocol.FLAG_HAS_TYPE_ID_INDEX.rawValue) != 0)
                current.compactId = -1
            } else {
                //
                // Only the most derived slice encodes the type ID for the compact format.
                //
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
        if (current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE.rawValue) != 0 {
            current.sliceSize = try stream.read()
            if current.sliceSize < 4 {
                throw UnmarshalOutOfBoundsException(reason: "invalid slice size")
            }
        } else {
            current.sliceSize = 0
        }

        return current.typeId
    }

    func endSlice() throws {
        if (current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue) != 0 {
            try stream.skipOptionals()
        }

        //
        // Read the indirection table if one is present and transform the
        // indirect patch list into patch entries with direct references.
        //
        if (current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE.rawValue) != 0 {
            var indirectionTable = [Int32](repeating: 0, count: Int(try stream.readAndCheckSeqSize(minSize: 1)))

            for i in 0 ..< indirectionTable.count {
                indirectionTable[i] = try readInstance(index: stream.readSize(), cb: nil)
            }

            //
            // Sanity checks. If there are optional members, it's possible
            // that not all instance references were read if they are from
            // unknown optional data members.
            //
            if indirectionTable.isEmpty {
                throw MarshalException(reason: "empty indirection table")
            }
            if current.indirectPatchList.isEmpty,
                (current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue) == 0 {
                throw MarshalException(reason: "no references to indirection table")
            }

            //
            // Convert indirect references into direct references.
            //
            for e in current.indirectPatchList {
                precondition(e.index >= 0)
                if e.index >= indirectionTable.count {
                    throw MarshalException(reason: "indirection out of range")
                }
                try addPatchEntry(index: indirectionTable[Int(e.index)], cb: e.cb)
            }
            current.indirectPatchList.removeAll()
        }
    }

    func skipSlice() throws {
        stream.traceSkipSlice(typeId: current.typeId, sliceType: current.sliceType)

        let start = stream.getSize()

        if (current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE.rawValue) != 0 {
            precondition(current.sliceSize >= 4)
            try stream.skip(current.sliceSize - 4)
        } else {
            if current.sliceType == .ValueSlice {
                throw NoValueFactoryException(reason: "no value factory found and compact format prevents " +
                    "slicing (the sender should use the sliced format instead)",
                                              type: current.typeId)
            } else {
                if let r = current.typeId.range(of: "::") {
                    throw UnknownUserException(unknown: String(current.typeId[r.upperBound...]))
                } else {
                    throw UnknownUserException(unknown: current.typeId)
                }
            }
        }

        //
        // Preserve this slice.
        //
        let hasOptionalMembers = (current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue) != 0
        let isLastSlice = (current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE.rawValue) != 0
        let buffer = stream.getBuffer()
        let end: Int = buffer.position()
        var dataEnd = end

        if hasOptionalMembers {
            //
            // Don't include the optional member end marker. It will be re-written by
            // endSlice when the sliced data is re-written.
            //
            dataEnd -= 1
        }

        try buffer.position(start)
        let bytes: [UInt8] = try Array(buffer.read(count: dataEnd - start))
        try buffer.position(end)

        let info = SliceInfo(typeId: current.typeId,
                             compactId: current.compactId,
                             bytes: bytes,
                             instances: [],
                             hasOptionalMembers: hasOptionalMembers,
                             isLastSlice: isLastSlice)

        //
        // Read the indirect instance table. We read the instances or their
        // IDs if the instance is a reference to an already unmarhsaled
        // instance.
        //
        // The SliceInfo object sequence is initialized only if
        // readSlicedData is called.
        //
        if (current.sliceFlags & Protocol.FLAG_HAS_INDIRECTION_TABLE.rawValue) != 0 {
            var indirectionTable = [Int32](repeating: 0, count: Int(try stream.readAndCheckSeqSize(minSize: 1)))

            for i in 0 ..< indirectionTable.count {
                indirectionTable[i] = try readInstance(index: stream.readSize(), cb: nil)
            }
            current.indirectionTables.append(indirectionTable)
        } else {
            current.indirectionTables.append([])
        }

        current.slices.append(info)
    }

    func readOptional(tag: Int32, format: OptionalFormat) throws -> Bool {
        if current == nil {
            return try stream.readOptionalImpl(readTag: tag, expectedFormat: format)
        } else if (current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue) != 0 {
            return try stream.readOptionalImpl(readTag: tag, expectedFormat: format)
        }
        return false
    }

    //swiftlint:disable cyclomatic_complexity
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

        var v: Value?
        while true {
            var updateCache = false

            if current.compactId >= 0 {
                updateCache = true

                //
                // Translate a compact (numeric) type ID into a class.
                //
                if !compactIdCache.isEmpty {
                    //
                    // Check the cache to see if we've already translated the compact type ID into a class.
                    //
                    if let cls: Value.Type = compactIdCache[current.compactId] {
                        v = cls.init()
                        updateCache = false
                    }
                }

                //
                // If we haven't already cached a class for the compact ID, then try to translate the
                // compact ID into a type ID.
                //
                if v == nil {
                    current.typeId = TypeIdResolver.resolve(compactId: current.compactId) ?? ""
                }
            }

            if v == nil, !current.typeId.isEmpty {
                v = try newInstance(typeId: current.typeId)
            }

            if let v = v {
                if updateCache {
                    precondition(current.compactId >= 0)
                    compactIdCache[current.compactId] = type(of: v)
                }

                //
                // We have an instance, get out of this loop.
                //
                break
            }

            //
            // If slicing is disabled, stop unmarshaling.
            //
            if !sliceValues {
                throw NoValueFactoryException(reason: "no value factory found and slicing is disabled",
                                              type: current.typeId)
            }

            //
            // Slice off what we don't understand.
            //
            try skipSlice()

            //
            // If this is the last slice, keep the instance as an opaque
            // UnknownSlicedValue object.
            //
            if (current.sliceFlags & Protocol.FLAG_IS_LAST_SLICE.rawValue) != 0 {
                //
                // Provide a factory with an opportunity to supply the instance.
                // We pass the "::Ice::Object" ID to indicate that this is the
                // last chance to preserve the instance.
                //
                v = try newInstance(typeId: "::Ice::Object")
                if v == nil {
                    v = UnknownSlicedValue(unknownTypeId: mostDerivedId)
                }

                break
            }

            _ = try startSlice() // Read next Slice header for next iteration.
        }

        //
        // Unmarshal the instance.
        //
        try unmarshal(index: index, v: v!)

        if current == nil, !patchMap.isEmpty {
            //
            // If any entries remain in the patch map, the sender has sent an index for an instance, but failed
            // to supply the instance.
            //
            throw MarshalException(reason: "index for class received, but no instance")
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

        for n in 0 ..< current.slices.count {
            //
            // We use the "instances" list in SliceInfo to hold references
            // to the target instances. Note that the instances might not have
            // been read yet in the case of a circular reference to an
            // enclosing instance.

            let m = n
            let sz = current.indirectionTables[m].count
            current.slices[m].instances = [Ice.Value?](repeating: nil, count: sz)
            for j in 0 ..< current.slices[m].instances.count {
                let k = j
                try addPatchEntry(index: current.indirectionTables[m][j]) { v in
                    self.current.slices[m].instances[k] = v
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
        self.values = [DictEntry<K, V>](repeating: DictEntry<K, V>(), count: size)
    }
}
