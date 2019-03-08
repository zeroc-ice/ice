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

    var capacity: Int {
        return storage.count
    }

    var size = 0

    var position: Int32 {
        return Int32(size)
    }

    var baseAddress: UnsafeMutableRawPointer? {
        return storage.baseAddress
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
        write(bytes: bytes, at: size)
        size += bytes.count
    }

    func skip(count: Int) throws {
        guard count + self.size <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        self.size += count
    }

    func position(_ count: Int) throws {
        guard count > capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        self.size = count
    }

    func load(bytes: UnsafeRawBufferPointer) throws {
        return try read(from: size, into: UnsafeMutableRawBufferPointer(mutating: bytes))
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
        guard from + size <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        let rebase = UnsafeRawBufferPointer(rebasing: storage[from ..< from + bytes.count])
        bytes.copyMemory(from: rebase)
        size += bytes.count
    }

    func read(count: Int) throws -> UnsafeRawBufferPointer {
        guard count + self.size <= capacity else {
            throw UnmarshalOutOfBoundsException(reason: "attempting to read past buffer capacity")
        }
        let rebase = UnsafeRawBufferPointer(rebasing: storage[self.size ..< self.size + count])
        self.size += count
        return rebase
    }

    func expand(capacity c: Int) {
        let bytes = UnsafeMutableRawBufferPointer.allocate(byteCount: c,
                                                           alignment: MemoryLayout<UInt8>.alignment)
        bytes.copyBytes(from: storage)
        storage.deallocate()
        storage = bytes
    }

    func ensure(bytesNeeded: Int) {
        if size + bytesNeeded > capacity {
            expand(capacity: max((capacity - bytesNeeded), 2 * capacity))
        }
    }

    private func slice(start: Int, count: Int) -> UnsafeMutableRawBufferPointer {
        return UnsafeMutableRawBufferPointer(rebasing: storage[start ..< start + count])
    }
}
