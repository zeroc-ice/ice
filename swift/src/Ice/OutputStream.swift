// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

public class OutputStream {
    private var buf: Buffer
    private var communicator: Communicator
    private var encoding: EncodingVersion
    private var format: FormatType = FormatType.DefaultFormat

    private var encapsStack: Encaps!
    private var encapsCache: Encaps?

    public init(communicator: Communicator, encoding: EncodingVersion = currentEncoding()) {
        self.communicator = communicator
        self.encoding = encoding
        buf = Buffer()

        if communicator.getProperties().getPropertyAsIntWithDefault(key: "Ice.Default.SlicedFormat", value: 0) > 0 {
            format = FormatType.SlicedFormat
        } else {
            format = FormatType.CompactFormat
        }
    }

    public func startEncapsulation() {
        //
        // If no encoding version is specified, use the current write
        // encapsulation encoding version if there's a current write
        // encapsulation, otherwise, use the stream encoding version.
        //
        if encapsStack != nil {
            startEncapsulation(encoding: encapsStack.encoding, format: encapsStack.format)
        } else {
            startEncapsulation(encoding: encoding, format: FormatType.DefaultFormat)
        }
    }

    public func startEncapsulation(encoding: EncodingVersion, format: FormatType) {
        var curr = encapsCache
        if let c = curr {
            c.reset()
            encapsCache = c.next
        } else {
            curr = Encaps()
        }
        curr!.next = encapsStack
        encapsStack = curr

        encapsStack.format = format
        encapsStack.setEncoding(encoding)
        encapsStack.start = buf.count

        Int32(0).ice_write(to: self) // Placeholder for the encapsulation length.
        encapsStack.encoding.ice_write(to: self)
    }

    public func endEncapsulation() {
        // Size includes size and version.
        let start = encapsStack.start
        var sz = Int32(buf.count - start)
        write(bytes: &sz, at: start)

        let curr = encapsStack!
        encapsStack = curr.next
        curr.next = encapsCache
        encapsCache = curr
        encapsCache!.reset()
    }

    func getBytes() -> UnsafeMutableRawPointer? {
        return buf.baseAddress
    }

    func getCount() -> Int {
        return buf.count
    }

    func write<T>(bytes value: inout T, at: Int) {
        withUnsafePointer(to: &value) {
            self.buf.write(bytes: UnsafeRawBufferPointer(start: $0, count: MemoryLayout<T>.size), at: at)
        }
    }

    func startValue(data: SlicedData?) {
        precondition(encapsStack.encoder != nil)
        encapsStack.encoder.startInstance(type: .ValueSlice, data: data)
    }

    func endValue() {
        precondition(encapsStack.encoder != nil)
        encapsStack.encoder.endInstance()
    }

    private func initEncaps() {
        // Lazy initialization.

        if let e = encapsCache {
            encapsStack = e
            encapsCache = encapsCache!.next
        } else {
            encapsStack = Encaps()
        }

        encapsStack.setEncoding(encoding)

        if encapsStack.format == FormatType.DefaultFormat {
            encapsStack.format = format
        }

        // Lazy initialization.
        if encapsStack.encoder == nil {
            if encapsStack.encoding_1_0 {
                encapsStack.encoder = EncapsEncoder10(os: self, encaps: encapsStack)
            } else {
                encapsStack.encoder = EncapsEncoder11(os: self, encaps: encapsStack)
            }
        }
    }

    public func finished() -> [UInt8] {
        // Create a copy
        return Array(UnsafeRawBufferPointer(start: buf.baseAddress!, count: buf.count))
    }
}

public extension OutputStream {
    func write(_ streamables: Streamable...) {
        for streamable in streamables {
            streamable.ice_write(to: self)
        }
    }

    func write<Element>(numeric n: Element) where Element: Streamable, Element: Numeric {
        var value = n
        return Swift.withUnsafeBytes(of: &value) {
            self.buf.append(bytes: $0)
        }
    }

    func write(size: Int32) {
        if size > 254 {
            // pre-allocate size memory
            buf.ensure(bytesNeeded: Int(5))
            write(numeric: UInt8(255))
            write(numeric: size)
        } else {
            // pre-allocate size memory
            buf.ensure(bytesNeeded: Int(1 + size))
            write(numeric: UInt8(size))
        }
    }

    func write(enum val: UInt8, maxValue: Int32) {
        if encoding == Protocol.Encoding_1_0 {
            if maxValue < 127 {
                write(numeric: UInt8(val))
            } else if maxValue < 32767 {
                write(numeric: Int16(val))
            } else {
                write(numeric: Int32(val))
            }
        } else {
            write(size: Int32(val))
        }
    }

    func write(enum val: Int32, maxValue: Int32) {
        if encoding == Protocol.Encoding_1_0 {
            if maxValue < 127 {
                write(numeric: UInt8(val))
            } else if maxValue < 32767 {
                write(numeric: Int16(val))
            } else {
                write(numeric: Int32(val))
            }
        } else {
            write(size: val)
        }
    }

    func write(utf8view view: String.UTF8View) {
        let byteArray = [UInt8](view)
        write(size: byteArray.count)
        byteArray.withUnsafeBytes { self.buf.append(bytes: $0) }
    }

    func write(size: Int) {
        precondition(size <= Int32.max, "Size is too large")
        write(size: Int32(size))
    }

    func write<Element>(array: [Element]) where Element: Streamable {
        write(numeric: Int32(array.count))
        for item in array {
            item.ice_write(to: self)
        }
    }

    func write(proxy: ObjectPrx?) throws {
        if let prxImpl = proxy as? _ObjectPrxI {
            try prxImpl.ice_write(to: self)
        } else {
            //
            // A nil proxy is represented by an Identity with empty name and category fields.
            //
            Ice.Identity().ice_write(to: self)
        }
    }

    func write(value: Value?) {
        initEncaps()
        encapsStack.encoder?.writeValue(v: value)
    }

    func write(proxyArray: [ObjectPrx?]) throws {
        write(numeric: Int32(proxyArray.count))
        for prx in proxyArray {
            try write(proxy: prx)
        }
    }
}

extension OutputStream: ICEOutputStreamHelper {
    public func copy(_ start: UnsafeRawPointer, count: NSNumber) {
        buf.append(bytes: UnsafeRawBufferPointer(start: start, count: count.intValue))
    }
}

private class Encaps {
    var start: Int = 0
    var format: FormatType = FormatType.DefaultFormat
    var encoding: EncodingVersion = Ice.currentEncoding()
    var encoding_1_0: Bool = false

    var encoder: EncapsEncoder!
    var next: Encaps?

    func reset() {
        encoder = nil
        next = nil
    }

    func setEncoding(_ encoding: EncodingVersion) {
        self.encoding = encoding
    }
}

private enum SliceType {
    case NoSlice
    case ValueSlice
    case ExceptionSlice
}

private struct ValueHolder: Hashable {
    let value: Value
    var hashValue: Int {
        return ObjectIdentifier(value).hashValue
    }

    init(_ value: Value) {
        self.value = value
    }

    static func == (lhs: ValueHolder, rhs: ValueHolder) -> Bool {
        return ObjectIdentifier(lhs.value) == ObjectIdentifier(rhs.value)
    }
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
    #warning("TODO: this is missing from MATLAB but is in JAVA and others")
//    func writeException(v: UserException)

    func startInstance(type: SliceType, data: SlicedData?)
    func endInstance()

    func startSlice(typeId: String, compactId: Int32, last: Bool)
    func endSlice()

    // Implemented for the 1.0 encoding, not necessary for subsequent encodings.
    func writePendingValues()
}

extension EncapsEncoder {
    func writeOptional(tag _: Int, format _: OptionalFormat) -> Bool {
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
    var os: OutputStream
    var encaps: Encaps
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
            os.write(numeric: -registerValue(val))
        } else {
            os.write(numeric: Int32(0))
        }
    }

//    func writeException(v: UserException) {
//        //
//        // User exception with the 1.0 encoding start with a boolean
//        // flag that indicates whether or not the exception uses
//        // classes.
//        //
//        // This allows reading the pending instances even if some part of
//        // the exception was sliced.
//        //
//        let usesClasses = v._usesClasses();
//    }

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
                false.ice_write(to: os)
                typeId.ice_write(to: os)
            } else {
                true.ice_write(to: os)
                os.write(size: index)
            }
        } else {
            typeId.ice_write(to: os)
        }

        os.write(numeric: Int32(0)) // Placeholder for the slice length.
        writeSlice = Int32(os.getCount())
    }

    func endSlice() {
        //
        // Write the slice length.
        //
        var sz = Int32(os.getCount()) - writeSlice + 4
        os.write(bytes: &sz, at: Int(writeSlice - 4))
    }

    func writePendingValues() {
        while !toBeMarshaledMap.isEmpty {
            //
            // Consider the to be marshalled instances as marshaled now,
            // this is necessary to avoid adding again the "to be
            // marshaled instances" into _toBeMarshaledMap while writing
            // instances.
            //
            toBeMarshaledMap.forEach { key, value in
                marshaledMap[key] = value
            }

            let savedMap = toBeMarshaledMap
            toBeMarshaledMap = [ValueHolder: Int32]()
            os.write(size: savedMap.count)

            savedMap.forEach { key, value in
                //
                // Consider the to be marshalled instances as marshaled now,
                // this is necessary to avoid adding again the "to be
                // marshaled instances" into _toBeMarshaledMap while writing
                // instances.
                //
                os.write(numeric: Int32(value))

                key.value.ice_preMarshal()

                key.value.ice_write(to: os)
            }
        }
        os.write(numeric: Int32(0)) // Zero marker indicates end of sequence of sequences of instances.
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
    var os: OutputStream
    var encaps: Encaps

    var marshaledMap: [ValueHolder: Int32]
    lazy var typeIdMap: [String: Int32] = [String: Int32]()
    var typeIdIndex: Int32

    var current: InstanceData!
    var valueIdIndex: Int32 = 0 // The ID of the next instance to marhsal

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

        if let curr = current, current != nil, encaps.format == FormatType.SlicedFormat {
            //
            // If writing an instance within a slice and using the sliced
            // format, write an index from the instance indirection
            // table. The indirect instance table is encoded at the end of
            // each slice and is always read (even if the Slice is
            // unknown).
            //
            let vh = ValueHolder(v)
            if let index = curr.indirectionMap[vh] {
                os.write(size: index)
            } else {
                curr.indirectionTable.append(vh)
                let idx = curr.indirectionTable.count // Position + 1 (0 is reserved for nil)
                curr.indirectionMap[vh] = Int32(idx)
                os.write(size: idx)
            }
        } else {
            writeInstance(v) // Write the instance or a reference if already marshaled.
        }
    }

//    func writeException(v: UserException) {
//        <#code#>
//    }

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
        #warning("TODO: add precondition failures. See Java/MATLAB/etc.")

        current.sliceFlagsPos = Int32(os.getCount())
        current.sliceFlags = 0

        if encaps.format == FormatType.SlicedFormat {
            // Encode the slice size if using the sliced format.
            current.sliceFlags |= Protocol.FLAG_HAS_SLICE_SIZE.rawValue
        }
        if last {
            current.sliceFlags |= Protocol.FLAG_IS_LAST_SLICE.rawValue // This is the last slice.
        }

        os.write(numeric: UInt8(0)) // Placeholder for the slice flags

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
                    current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_COMPACT.rawValue
                    os.write(size: compactId)
                } else {
                    let index = registerTypeId(typeId)
                    if index < 0 {
                        current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_STRING.rawValue
                        typeId.ice_write(to: os)
                    } else {
                        current.sliceFlags |= Protocol.FLAG_HAS_TYPE_ID_INDEX.rawValue
                        os.write(size: index)
                    }
                }
            }
        } else {
            typeId.ice_write(to: os)
        }

        if (current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE.rawValue) != 0 {
            os.write(numeric: Int32(0)) // Placeholder for the slice length.
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
        if (current.sliceFlags & Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue) != 0 {
            Protocol.OPTIONAL_END_MARKER.rawValue.ice_write(to: os)
        }

        //
        // Write the slice length if necessary.
        //
        if (current.sliceFlags & Protocol.FLAG_HAS_SLICE_SIZE.rawValue) != 0 {
            var sz: Int32 = Int32(os.getCount()) - current.writeSlice + 4
            os.write(bytes: &sz, at: Int(current.writeSlice - 4))
        }

        //
        // Only write the indirection table if it contains entries.
        //
        if !current.indirectionTable.isEmpty {
            precondition(encaps.format == FormatType.SlicedFormat)
            current.sliceFlags |= Protocol.FLAG_HAS_INDIRECTION_TABLE.rawValue

            //
            // Write the indirection instance table.
            //
            os.write(size: current.indirectionTable.count)
            current.indirectionTable.forEach { v in
                writeInstance(v.value)
            }

            current.indirectionTable.removeAll()
            current.indirectionMap.removeAll()
        }

        //
        // Finally, update the slice flags.
        //
        os.write(bytes: &current.sliceFlags, at: Int(current.sliceFlagsPos))
    }

    func writeSlicedData(_ slicedData: SlicedData) {
        //
        // We only remarshal preserved slices if we are using the sliced
        // format. Otherwise, we ignore the preserved slices, which
        // essentially "slices" the instance into the most-derived type
        // known by the sender.
        //
        guard encaps.format == FormatType.SlicedFormat else {
            return
        }

        slicedData.slices.forEach { info in
            startSlice(typeId: info.typeId, compactId: info.compactId, last: info.isLastSlice)

            //
            // Write the bytes associated with this slice.
            //
            info.bytes.ice_write(to: os)

            if info.hasOptionalMembers {
                current.sliceFlags |= Protocol.FLAG_HAS_OPTIONAL_MEMBERS.rawValue
            }

            //
            // Make sure to also re-write the instance indirection table.
            //
            info.instances.forEach { o in
                current.indirectionTable.append(ValueHolder(o))
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
        v.ice_write(to: os)
    }
}

private class InstanceData {
    // Instance attributes
    var sliceType: SliceType = SliceType.NoSlice
    var firstSlice: Bool = true

    // Slice attributes
    var sliceFlags: UInt8 = 0
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
