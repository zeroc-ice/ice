// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

internal final class Buffer {
    private var storage: UnsafeMutableRawBufferPointer
    private let owner: Bool

    private var _position: Int = 0

    var capacity: Int {
        return storage.count
    }

    var baseAddress: UnsafeMutableRawPointer? {
        return storage.baseAddress
    }

    var constBaseAddress: UnsafeRawPointer? {
        return UnsafeRawBufferPointer(storage).baseAddress
    }

    init(start: UnsafeMutableRawPointer, count: Int) {
        storage = UnsafeMutableRawBufferPointer(start: start, count: count)
        owner = false
    }

    init(count: Int = 240) {
        storage = UnsafeMutableRawBufferPointer.allocate(byteCount: count, alignment: MemoryLayout<UInt8>.alignment)
        owner = true
    }

    deinit {
        if owner {
            self.storage.deallocate()
        }
    }

    func append(bytes: UnsafeRawBufferPointer) {
        ensure(bytesNeeded: bytes.count)
        write(bytes: bytes, at: _position)
        _position += bytes.count
    }

    func skip<T>(_ count: T) throws where T: BinaryInteger {
        let c = Int(count)
        //
        // Skip is allowed to jump to the "end" of the buffer (c + position == capacity)
        // No more bytes can be read after this
        //
        guard count > 0, c + _position <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to set position outside buffer")
        }
        _position += c
    }

    func position<T>() -> T where T: BinaryInteger {
        return T(_position)
    }

    func position<T>(_ count: T) throws where T: BinaryInteger {
        //
        // Position is allowed to jump to the "end" of the buffer (count == capacity)
        // No more bytes can be read after this
        //
        guard count >= 0, count <= capacity  else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to set position outiside buffer")
        }
        _position = Int(count)
    }

    func load(bytes: UnsafeRawBufferPointer) throws {
        return try read(from: _position, into: UnsafeMutableRawBufferPointer(mutating: bytes))
    }

    func load<T>(as _: T.Type) throws -> T {
        return try read(count: MemoryLayout<T>.size).baseAddress!.bindMemory(to: T.self, capacity: 1).pointee
    }

    func write(bytes: UnsafeRawBufferPointer, at index: Int) {
        precondition(index + bytes.count <= capacity,
                     "Buffer index + count ( \(index) + \(bytes.count) is greather than capacity (\(capacity))")

        let target = slice(start: index, count: bytes.count)
        target.copyMemory(from: bytes)
    }

    func read(from: Int, into bytes: UnsafeMutableRawBufferPointer) throws {
        guard from + _position <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        let rebase = UnsafeRawBufferPointer(rebasing: storage[from ..< from + bytes.count])
        bytes.copyMemory(from: rebase)
        _position += bytes.count
    }

    func read(count: Int) throws -> UnsafeRawBufferPointer {
        guard count + _position <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        let rebase = UnsafeRawBufferPointer(rebasing: storage[_position ..< _position + count])
        _position += count
        return rebase
    }

    func expand(bytesNeeded: Int) {
        precondition(owner, "can only expand owned buffer")

        guard _position + bytesNeeded > capacity else {
            return
        }

        let bytes = UnsafeMutableRawBufferPointer.allocate(byteCount: max(_position + bytesNeeded, capacity * 2),
                                                           alignment: MemoryLayout<UInt8>.alignment)
        bytes.copyBytes(from: storage)
        storage.deallocate()
        storage = bytes
    }

    func ensure(bytesNeeded: Int) {
        if _position + bytesNeeded > capacity {
            expand(bytesNeeded: bytesNeeded)
        }
    }

    private func slice(start: Int, count: Int) -> UnsafeMutableRawBufferPointer {
        return UnsafeMutableRawBufferPointer(rebasing: storage[start ..< start + count])
    }
}
