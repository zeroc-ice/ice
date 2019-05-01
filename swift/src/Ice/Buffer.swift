//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation

internal final class Buffer {
    var data: Data
    private var pos: Int = 0

    var capacity: Int {
        return data.count
    }

    var remaining: Int {
        return capacity - pos
    }

    init(_ data: Data = Data(capacity: 240)) {
       self.data = data
    }

    func append<T>(_ value: T) where T: Numeric {
        withUnsafePointer(to: value) { ptr in
            self.data.append(UnsafeBufferPointer<T>(start: ptr, count: 1))
        }
        pos += MemoryLayout<T>.size
    }

    func append(bytes: [UInt8]) {
        data.append(contentsOf: bytes)
        pos += bytes.count
    }

    func append(bytes: Data) {
        data.append(bytes)
        pos += bytes.count
    }

    func skip<T>(_ count: T) throws where T: BinaryInteger {
        let c = Int(count)
        //
        // Skip is allowed to jump to the "end" of the buffer (c + position == capacity)
        // No more bytes can be read after this
        //
        guard count >= 0, c + pos <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to set position outside buffer")
        }
        pos += c
    }

    func position<T>() -> T where T: BinaryInteger {
        return T(pos)
    }

    func position<T>(_ count: T) throws where T: BinaryInteger {
        //
        // pos is allowed to jump to the "end" of the buffer (count == capacity)
        // No more bytes can be read after this
        //
        guard count >= 0, count <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to set position outside buffer")
        }
        pos = Int(count)
    }

    func load<T>(as _: T.Type) throws -> T {
        let count = MemoryLayout<T>.size
        guard count + pos <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        return data[pos ..< pos + count].withUnsafeBytes { ptr in
            let result = ptr.baseAddress!.bindMemory(to: T.self, capacity: 1).pointee
            pos += count
            return result
        }
    }
}
