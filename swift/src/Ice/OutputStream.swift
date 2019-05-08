//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import IceObjc

public class OutputStream {
    private var data: Data = Data(capacity: 240)
    private let communicator: Communicator
    private let encoding: EncodingVersion
    private let encoding_1_0: Bool
    private let format: FormatType

    private var encaps: Encaps!

    var currentEncoding: EncodingVersion {
        return encaps != nil ? encaps.encoding : encoding
    }

    public convenience init(communicator: Communicator) {
        let encoding = (communicator as! CommunicatorI).defaultsAndOverrides.defaultEncoding
        self.init(communicator: communicator, encoding: encoding)
    }

    public init(communicator: Communicator, encoding: EncodingVersion) {
        self.communicator = communicator
        self.encoding = encoding
        encoding_1_0 = encoding == Encoding_1_0
        format = (communicator as! CommunicatorI).defaultsAndOverrides.defaultFormat
    }

    public func startEncapsulation() {
        startEncapsulation(encoding: encoding, format: FormatType.DefaultFormat)
    }

    public func startEncapsulation(encoding: EncodingVersion, format: FormatType) {
        precondition(encaps == nil, "Nested or sequential encapsulations are not supported")
        encaps = Encaps(encoding: encoding, format: format, start: data.count)
        write(Int32(0)) // Placeholder for the encapsulation length.
        write(encaps.encoding)
    }

    public func endEncapsulation() {
        // Size includes size and version.
        let start = encaps.start
        let sz = Int32(data.count - start)
        write(bytesOf: sz, at: start)
    }

    func writeEmptyEncapsulation(_ encoding: EncodingVersion) {
        write(Int32(6)) // Size
        write(encoding)
    }

    func writeEncapsulation(_ v: Data) {
        precondition(v.count >= 6, "Encapsulation is invalid. Size is too small.")
        data.append(v)
    }

    func getCount() -> Int {
        return data.count
    }

    //
    // Overwrite an existing Numeric at the specified position
    //
    func write<Element>(bytesOf value: Element, at: Int) where Element: StreamableNumeric {
        withUnsafePointer(to: value) { ptr in
            self.data.replaceSubrange(at ..< at + MemoryLayout<Element>.size,
                                      with: UnsafeRawPointer(ptr), count: MemoryLayout<Element>.size)
        }
    }

    public func startValue(data: SlicedData?) {
        precondition(encaps.encoder != nil)
        encaps.encoder.startInstance(type: .ValueSlice, data: data)
    }

    public func endValue() {
        precondition(encaps.encoder != nil)
        encaps.encoder.endInstance()
    }

    public func startException(data: SlicedData?) {
        precondition(encaps.encoder != nil)
        encaps.encoder.startInstance(type: .ExceptionSlice, data: data)
    }

    public func endException() {
        precondition(encaps.encoder != nil)
        encaps.encoder.endInstance()
    }

    private func initEncaps() {
        if encaps == nil {
            encaps = Encaps(encoding: encoding, format: format, start: 0)
        } else if encaps.format == .DefaultFormat {
            encaps.format = format
        }

        // Lazy initialization.
        if encaps.encoder == nil {
            if encaps.encoding_1_0 {
                encaps.encoder = EncapsEncoder10(os: self, encaps: encaps)
            } else {
                encaps.encoder = EncapsEncoder11(os: self, encaps: encaps)
            }
        }
    }

    public func writePendingValues() {
        if encaps != nil, encaps.encoder != nil {
            encaps.encoder.writePendingValues()
        } else if encoding_1_0 {
            // If using the 1.0 encoding and no instances were written, we
            // still write an empty sequence for pending instances if
            // requested (i.e.: if this is called).
            //
            // This is required by the 1.0 encoding, even if no instances
            // are written we do marshal an empty sequence if marshaled
            // data types use classes.
            write(size: Int32(0))
        }
    }

    // Returns the underlying data
    public func finished() -> Data {
        return data
    }

    public func startSlice(typeId: String, compactId: Int32, last: Bool) {
        precondition(encaps != nil && encaps.encoder != nil)
        encaps.encoder.startSlice(typeId: typeId, compactId: compactId, last: last)
    }

    public func endSlice() {
        precondition(encaps != nil && encaps.encoder != nil)
        encaps.encoder.endSlice()
    }
}

public extension OutputStream {
    //
    // StreamableNumeric
    //
    func write<Element>(_ v: Element) where Element: StreamableNumeric {
        // We assume a little-endian platform
        withUnsafePointer(to: v) { ptr in
            self.data.append(UnsafeBufferPointer<Element>(start: ptr, count: 1))
        }
    }

    func write<Element>(tag: Int32, value: Element?) where Element: StreamableNumeric {
        let format = OptionalFormat(fixedSize: MemoryLayout<Element>.size)
        if let val = value {
            if writeOptional(tag: tag, format: format!) {
                write(val)
            }
        }
    }

    func write<Element>(_ v: [Element]) where Element: StreamableNumeric {
        write(size: v.count)

        if v.count <= 1 || MemoryLayout<Element>.size == MemoryLayout<Element>.stride {
            v.withUnsafeBufferPointer { buf in
                self.data.append(buf)
            }
        } else {
            for e in v {
                write(e)
            }
        }
    }

    func write<Element>(tag: Int32, value: [Element]?) where Element: StreamableNumeric {
        if let val = value {
            if writeOptionalVSize(tag: tag, len: val.count, elemSize: MemoryLayout<Element>.size) {
                write(val)
            }
        }
    }

    //
    // UInt8 optimization
    //
    func write(_ v: UInt8) {
        data.append(v)
    }

    func write(_ v: [UInt8]) {
        write(size: v.count)
        if v.count > 0 {
            data.append(contentsOf: v)
        }
    }

    func write(_ v: Data) {
        write(size: v.count)
        if v.count > 0 {
            data.append(v)
        }
    }

    func write(tag: Int32, value: Data?) {
        if let val = value {
            // Note: not the same as larger Numeric
            if writeOptional(tag: tag, format: .VSize) {
                write(val)
            }
        }
    }

    //
    // Bool
    //
    func write(_ v: Bool) {
        write(UInt8(v == true ? 1 : 0))
    }

    func write(tag: Int32, value: Bool?) {
        if let val = value {
            if writeOptional(tag: tag, format: .F1) {
                write(val)
            }
        }
    }

    func write(_ v: [Bool]) {
        write(size: v.count)
        if MemoryLayout<Bool>.size == 1, MemoryLayout<Bool>.stride == 1 {
            v.withUnsafeBufferPointer { buf in
                self.data.append(buf)
            }
        } else {
            fatalError("Unsupported Bool layout")
        }
    }

    func write(tag: Int32, value: [Bool]?) {
        if let val = value {
            if writeOptional(tag: tag, format: .VSize) {
                write(val)
            }
        }
    }

    //
    // Size
    //
    func write(size: Int32) {
        if size > 254 {
            write(UInt8(255))
            write(size)
        } else {
            write(UInt8(size))
        }
    }

    func write(size: Int) {
        precondition(size <= Int32.max, "Size is too large")
        write(size: Int32(size))
    }

    func startSize() -> Int32 {
        let pos = Int32(data.count)
        write(Int32(0)) // Placeholder for 32-bit size
        return pos
    }

    func endSize(position: Int32) {
        precondition(position > 0)
        write(bytesOf: Int32(data.count) - position - 4, at: Int(position))
    }

    //
    // Enum
    //
    func write(enum val: UInt8, maxValue: Int32) {
        if currentEncoding == Encoding_1_0 {
            if maxValue < 127 {
                write(UInt8(val))
            } else if maxValue < 32767 {
                write(Int16(val))
            } else {
                write(Int32(val))
            }
        } else {
            write(size: Int32(val))
        }
    }

    func write(tag: Int32, val: UInt8, maxValue: Int32) {
        if writeOptional(tag: tag, format: .Size) {
            write(enum: val, maxValue: maxValue)
        }
    }

    func write(enum val: Int32, maxValue: Int32) {
        if currentEncoding == Encoding_1_0 {
            if maxValue < 127 {
                write(UInt8(val))
            } else if maxValue < 32767 {
                write(Int16(val))
            } else {
                write(Int32(val))
            }
        } else {
            write(size: val)
        }
    }

    func write(tag: Int32, val: Int32, maxValue: Int32) {
        if writeOptional(tag: tag, format: .Size) {
            write(enum: val, maxValue: maxValue)
        }
    }

    //
    // String
    //
    func write(_ v: String) {
        let bytes = v.data(using: .utf8)!
        write(size: bytes.count)
        data.append(bytes)
    }

    func write(tag: Int32, value v: String?) {
        if let val = v {
            if writeOptional(tag: tag, format: .VSize) {
                write(val)
            }
        }
    }

    func write(_ v: [String]) {
        write(size: v.count)
        for s in v {
            write(s)
        }
    }

    func write(tag: Int32, value v: [String]?) {
        if let val = v {
            if writeOptional(tag: tag, format: .FSize) {
                let pos = startSize()
                write(val)
                endSize(position: pos)
            }
        }
    }

    //
    // Proxy
    //
    func write(_ v: ObjectPrx?) {
        if let prxImpl = v as? ObjectPrxI {
            prxImpl.ice_write(to: self)
        } else {
            //
            // A nil proxy is represented by an Identity with empty name and category fields.
            //
            write(Identity())
        }
    }

    func write(tag: Int32, value v: ObjectPrx?) {
        if let val = v {
            if writeOptional(tag: tag, format: .FSize) {
                let pos = startSize()
                write(val)
                endSize(position: pos)
            }
        }
    }

    //
    // Value
    //
    func write(_ v: Value?) {
        initEncaps()
        encaps.encoder.writeValue(v: v)
    }

    func write(tag: Int32, value v: Value?) {
        if let val = v {
            if writeOptional(tag: tag, format: .Class) {
                write(val)
            }
        }
    }

    //
    // UserException
    //
    func write(_ v: UserException) {
        initEncaps()
        encaps.encoder.writeException(v: v)
    }

    func writeOptional(tag: Int32, format: OptionalFormat) -> Bool {
        precondition(encaps != nil)
        if let encoder = encaps.encoder {
            return encoder.writeOptional(tag: tag, format: format)
        }
        return writeOptionalImpl(tag: tag, format: format)
    }

    internal func writeOptionalImpl(tag: Int32, format: OptionalFormat) -> Bool {
        guard encoding != Encoding_1_0 else {
            return false
        }

        var v = format.rawValue
        if tag < 30 {
            v |= UInt8(tag) << 3
            write(v)

        } else {
            v |= 0x0F0 // tag = 30
            write(v)
            write(size: tag)
        }
        return true
    }

    func writeOptionalVSize(tag: Int32, len: Int, elemSize: Int) -> Bool {
        if writeOptional(tag: tag, format: .VSize) {
            if elemSize > 1 {
                // We optimize-out the size when elemSize == 1
                write(size: len == 0 ? 1 : (len * elemSize) + (len > 254 ? 5 : 1))
            }
            return true
        }
        return false
    }

    //
    // Raw Bytes
    //
    func write(raw: Data) {
        data.append(raw)
    }
}

extension OutputStream: ICEOutputStreamHelper {
    public func copy(_ bytes: Data) {
        data.append(bytes)
    }
}

private class Encaps {
    let start: Int
    var format: FormatType
    let encoding: EncodingVersion
    let encoding_1_0: Bool

    var encoder: EncapsEncoder!

    init(encoding: EncodingVersion, format: FormatType, start: Int) {
        self.start = start
        self.format = format
        self.encoding = encoding
        encoding_1_0 = encoding == Encoding_1_0
    }
}

private enum SliceType {
    case NoSlice
    case ValueSlice
    case ExceptionSlice
}

private struct ValueHolder: Hashable {
    init(_ value: Value) {
        self.value = value
    }

    func hash(into hasher: inout Hasher) {
        hasher.combine(ObjectIdentifier(value).hashValue)
    }

    static func == (lhs: ValueHolder, rhs: ValueHolder) -> Bool {
        return lhs.value === rhs.value
    }

    fileprivate let value: Value
}

private protocol EncapsEncoder: AnyObject {
    var os: OutputStream { get }
    var encaps: Encaps { get }

    // Encapsulation attributes for instance marshaling.
    var marshaledMap: [ValueHolder: Int32] { get }
    var typeIdMap: [String: Int32] { get set }
    var typeIdIndex: Int32 { get set }

    init(os: OutputStream, encaps: Encaps)

    func writeValue(v: Value?)
    func writeException(v: UserException)

    func startInstance(type: SliceType, data: SlicedData?)
    func endInstance()

    func startSlice(typeId: String, compactId: Int32, last: Bool)
    func endSlice()

    // Implemented for the 1.0 encoding, not necessary for subsequent encodings.
    func writePendingValues()
    func writeOptional(tag: Int32, format: OptionalFormat) -> Bool
}

extension EncapsEncoder {
    func writeOptional(tag _: Int32, format _: OptionalFormat) -> Bool {
        return false
    }

    func writePendingValues() {}

    func registerTypeId(_ typeId: String) -> Int32 {
        guard let p = typeIdMap[typeId] else {
            typeIdIndex += 1
            typeIdMap[typeId] = typeIdIndex
            return -1
        }
        return p
    }
}

private final class EncapsEncoder10: EncapsEncoder {
    unowned let os: OutputStream
    unowned let encaps: Encaps
    var marshaledMap: [ValueHolder: Int32]
    lazy var typeIdMap: [String: Int32] = [String: Int32]()
    var typeIdIndex: Int32

    // Instance attributes
    private var sliceType: SliceType = SliceType.NoSlice
    // Slice attributes
    private var writeSlice: Int32 = 0 // Position of the slice data members
    // Encapsulation attributes for instance marshaling.
    private var valueIdIndex: Int32 = 0
    private var toBeMarshaledMap = [ValueHolder: Int32]()

    init(os: OutputStream, encaps: Encaps) {
        self.os = os
        self.encaps = encaps
        marshaledMap = [ValueHolder: Int32]()
        typeIdIndex = 0
    }

    func writeValue(v: Value?) {
        //
        // Value references are encoded as a negative integer in 1.0.
        //
        if let val = v {
            os.write(-registerValue(val))
        } else {
            os.write(Int32(0))
        }
    }

    func writeException(v: UserException) {
        //
        // User exception with the 1.0 encoding start with a boolean
        // flag that indicates whether or not the exception uses
        // classes.
        //
        // This allows reading the pending instances even if some part of
        // the exception was sliced.
        //
        let usesClasses = v._usesClasses()
        os.write(usesClasses)
        v._iceWrite(to: os)
        if usesClasses {
            writePendingValues()
        }
    }

    func startInstance(type: SliceType, data _: SlicedData?) {
        sliceType = type
    }

    func endInstance() {
        if sliceType == SliceType.ValueSlice {
            //
            // Write the Object slice.
            //
            startSlice(typeId: "::Ice::Object", compactId: -1, last: true)
            os.write(size: 0) // For compatibility with the old AFM.
            endSlice()
        }
        sliceType = SliceType.NoSlice
    }

    func startSlice(typeId: String, compactId _: Int32, last _: Bool) {
        //
        // For instance slices, encode a boolean to indicate how the type ID
        // is encoded and the type ID either as a string or index. For
        // exception slices, always encode the type ID as a string.
        //
        if sliceType == SliceType.ValueSlice {
            let index = registerTypeId(typeId)
            if index < 0 {
                os.write(false)
                os.write(typeId)
            } else {
                os.write(true)
                os.write(size: index)
            }
        } else {
            os.write(typeId)
        }

        os.write(Int32(0)) // Placeholder for the slice length.
        writeSlice = Int32(os.getCount())
    }

    func endSlice() {
        //
        // Write the slice length.
        //
        let sz = Int32(os.getCount()) - writeSlice + 4
        os.write(bytesOf: sz, at: Int(writeSlice - 4))
    }

    func writePendingValues() {
        while !toBeMarshaledMap.isEmpty {
            //
            // Consider the to be marshalled instances as marshaled now,
            // this is necessary to avoid adding again the "to be
            // marshaled instances" into _toBeMarshaledMap while writing
            // instances.
            //
            for (key, value) in toBeMarshaledMap {
                marshaledMap[key] = value
            }

            let savedMap = toBeMarshaledMap
            toBeMarshaledMap = [ValueHolder: Int32]()
            os.write(size: savedMap.count)

            for (key, value) in savedMap {
                //
                // Consider the to be marshalled instances as marshaled now,
                // this is necessary to avoid adding again the "to be
                // marshaled instances" into _toBeMarshaledMap while writing
                // instances.
                //
                os.write(Int32(value))

                key.value.ice_preMarshal()

                key.value._iceWrite(to: os)
            }
        }
        os.write(size: 0) // Zero marker indicates end of sequence of sequences of instances.
    }

    func registerValue(_ v: Value) -> Int32 {
        //
        // Look for this instance in the to-be-marshaled map.
        //
        let val = ValueHolder(v)
        if let p = toBeMarshaledMap[val] {
            return p
        }

        //
        // Didn't find it, try the marshaled map next.
        //
        if let p = marshaledMap[val] {
            return p
        }

        //
        // We haven't seen this instance previously, create a new
        // index, and insert it into the to-be-marshaled map.
        //
        valueIdIndex += 1
        toBeMarshaledMap[val] = valueIdIndex
        return valueIdIndex
    }
}

private final class EncapsEncoder11: EncapsEncoder {
    unowned let os: OutputStream
    unowned let encaps: Encaps

    var marshaledMap: [ValueHolder: Int32]
    lazy var typeIdMap: [String: Int32] = [String: Int32]()
    var typeIdIndex: Int32

    var current: InstanceData!
    var valueIdIndex: Int32 = 1 // The ID of the next instance to marhsal

    init(os: OutputStream, encaps: Encaps) {
        self.os = os
        self.encaps = encaps
        marshaledMap = [ValueHolder: Int32]()
        typeIdIndex = 0
    }

    func writeValue(v: Value?) {
        guard let v = v else {
            os.write(size: 0)
            return
        }

        if let current = current, encaps.format == FormatType.SlicedFormat {
            //
            // If writing an instance within a slice and using the sliced
            // format, write an index from the instance indirection
            // table. The indirect instance table is encoded at the end of
            // each slice and is always read (even if the Slice is
            // unknown).
            //
            let vh = ValueHolder(v)
            if let index = current.indirectionMap[vh] {
                os.write(size: index)
            } else {
                current.indirectionTable.append(vh)
                let idx = current.indirectionTable.count // Position + 1 (0 is reserved for nil)
                current.indirectionMap[vh] = Int32(idx)
                os.write(size: idx)
            }
        } else {
            writeInstance(v) // Write the instance or a reference if already marshaled.
        }
    }

    func writeException(v: UserException) {
        v._iceWrite(to: os)
    }

    func startInstance(type: SliceType, data: SlicedData?) {
        if let curr = current {
            current = curr.next ?? InstanceData(previous: curr)
        } else {
            current = InstanceData(previous: nil)
        }

        current!.sliceType = type
        current!.firstSlice = true

        if let d = data {
            writeSlicedData(d)
        }
    }

    func endInstance() {
        current = current!.previous
    }

    func startSlice(typeId: String, compactId: Int32, last: Bool) {
        guard let current = current else {
            preconditionFailure("current is nil")
        }

        precondition(current.indirectionTable.isEmpty && current.indirectionMap.isEmpty)

        current.sliceFlagsPos = Int32(os.getCount())
        current.sliceFlags = []

        if encaps.format == FormatType.SlicedFormat {
            // Encode the slice size if using the sliced format.
            current.sliceFlags.insert(.FLAG_HAS_SLICE_SIZE)
        }
        if last {
            current.sliceFlags.insert(.FLAG_IS_LAST_SLICE) // This is the last slice.
        }

        os.write(UInt8(0)) // Placeholder for the slice flags

        //
        // For instance slices, encode the flag and the type ID either as a
        // string or index. For exception slices, always encode the type
        // ID a string.
        //
        if current.sliceType == SliceType.ValueSlice {
            //
            // Encode the type ID (only in the first slice for the compact
            // encoding).
            //
            if encaps.format == FormatType.SlicedFormat || current.firstSlice {
                if compactId >= 0 {
                    current.sliceFlags.insert(.FLAG_HAS_TYPE_ID_COMPACT)
                    os.write(size: compactId)
                } else {
                    let index = registerTypeId(typeId)
                    if index < 0 {
                        current.sliceFlags.insert(.FLAG_HAS_TYPE_ID_STRING)
                        os.write(typeId)
                    } else {
                        current.sliceFlags.insert(.FLAG_HAS_TYPE_ID_INDEX)
                        os.write(size: index)
                    }
                }
            }
        } else {
            os.write(typeId)
        }

        if current.sliceFlags.contains(.FLAG_HAS_SLICE_SIZE) {
            os.write(Int32(0)) // Placeholder for the slice length.
        }

        current.writeSlice = Int32(os.getCount())
        current.firstSlice = false
    }

    func endSlice() {
        guard let current = current else {
            preconditionFailure("current is nil")
        }
        //
        // Write the optional member end marker if some optional members
        // were encoded. Note that the optional members are encoded before
        // the indirection table and are included in the slice size.
        //
        if current.sliceFlags.contains(.FLAG_HAS_OPTIONAL_MEMBERS) {
            os.write(SliceFlags.OPTIONAL_END_MARKER.rawValue)
        }

        //
        // Write the slice length if necessary.
        //
        if current.sliceFlags.contains(.FLAG_HAS_SLICE_SIZE) {
            let sz: Int32 = Int32(os.getCount()) - current.writeSlice + 4
            os.write(bytesOf: sz, at: Int(current.writeSlice - 4))
        }

        //
        // Only write the indirection table if it contains entries.
        //
        if !current.indirectionTable.isEmpty {
            precondition(encaps.format == FormatType.SlicedFormat)
            current.sliceFlags.insert(.FLAG_HAS_INDIRECTION_TABLE)

            //
            // Write the indirection instance table.
            //
            os.write(size: current.indirectionTable.count)
            for v in current.indirectionTable {
                writeInstance(v.value)
            }

            current.indirectionTable.removeAll()
            current.indirectionMap.removeAll()
        }

        //
        // Finally, update the slice flags.
        //
        os.write(bytesOf: current.sliceFlags.rawValue, at: Int(current.sliceFlagsPos))
    }

    func writeOptional(tag: Int32, format: OptionalFormat) -> Bool {
        guard let current = current else {
            return os.writeOptionalImpl(tag: tag, format: format)
        }

        if os.writeOptionalImpl(tag: tag, format: format) {
            current.sliceFlags.insert(.FLAG_HAS_OPTIONAL_MEMBERS)
            return true
        } else {
            return false
        }
    }

    func writeSlicedData(_ slicedData: SlicedData) {
        //
        // We only remarshal preserved slices if we are using the sliced
        // format. Otherwise, we ignore the preserved slices, which
        // essentially "slices" the instance into the most-derived type
        // known by the sender.
        //
        guard encaps.format == .SlicedFormat else {
            return
        }

        for info in slicedData.slices {
            startSlice(typeId: info.typeId, compactId: info.compactId, last: info.isLastSlice)

            //
            // Write the bytes associated with this slice.
            //
            os.write(raw: info.bytes)

            if info.hasOptionalMembers {
                current.sliceFlags.insert(.FLAG_HAS_OPTIONAL_MEMBERS)
            }

            //
            // Make sure to also re-write the instance indirection table.
            //
            for o in info.instances {
                current.indirectionTable.append(ValueHolder(o!))
            }

            endSlice()
        }
    }

    func writeInstance(_ v: Value) {
        //
        // If the instance was already marshaled, just write it's ID.
        //
        if let p = marshaledMap[ValueHolder(v)] {
            os.write(size: p)
            return
        }

        //
        // We haven't seen this instance previously, create a new ID,
        // insert it into the marshaled map, and write the instance.
        //
        valueIdIndex += 1
        marshaledMap[ValueHolder(v)] = valueIdIndex

        v.ice_preMarshal()
        os.write(size: 1) // Class instance marker.
        v._iceWrite(to: os)
    }
}

private class InstanceData {
    // Instance attributes
    var sliceType: SliceType = SliceType.NoSlice
    var firstSlice: Bool = true

    // Slice attributes
    var sliceFlags: SliceFlags = []
    var writeSlice: Int32 = 0 // Position of the slice data members
    var sliceFlagsPos: Int32 = 0 // Position of the slice flags
    lazy var indirectionTable = [ValueHolder]()
    lazy var indirectionMap = [ValueHolder: Int32]()

    let previous: InstanceData?
    var next: InstanceData?

    init(previous: InstanceData?) {
        self.previous = previous
        next = nil
        if let p = previous {
            p.next = self
        }
    }
}
