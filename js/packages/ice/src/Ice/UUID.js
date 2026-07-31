// Copyright (c) ZeroC, Inc.

/**
 * Generates a version 4 (random) UUID.
 * @returns The UUID, in the canonical 8-4-4-4-12 lowercase hexadecimal form.
 */
export function generateUUID() {
    // crypto.getRandomValues is the only member of the Crypto interface that is not restricted to secure contexts,
    // so this works in browsers served over plain HTTP as well.
    const bytes = crypto.getRandomValues(new Uint8Array(16));
    // Set the version (4) and variant (10xx) bits.
    bytes[6] = (bytes[6] & 0x0f) | 0x40;
    bytes[8] = (bytes[8] & 0x3f) | 0x80;

    const hex = Array.from(bytes, byte => byte.toString(16).padStart(2, "0")).join("");
    return `${hex.slice(0, 8)}-${hex.slice(8, 12)}-${hex.slice(12, 16)}-${hex.slice(16, 20)}-${hex.slice(20)}`;
}
