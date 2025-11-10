// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        type HashMapKey =
            | bigint
            | number
            | string
            | boolean
            | {
                  equals(other: any): boolean;
                  hashCode(): number;
              };

        class HashMap<Key extends HashMapKey, Value> {
            /**
             * Creates an empty HashMap that uses the operator === for key and value comparisons.
             */
            constructor();

            /**
             * Creates a copy of the specified HashMap.
             *
             * @param other - The HashMap to copy.
             */
            constructor(other: HashMap<Key, Value>);

            /**
             * Creates an empty HashMap that uses the specified key comparator for key comparisons.
             *
             * @param keyComparator - A function that compares two keys for equality.
             */
            constructor(keyComparator: (k1: Key, k2: Key) => boolean);

            /**
             * Adds a new entry with a specified key and value to this Map, or updates an existing entry if the key
             * already exists.
             *
             * @param key The key of the entry.
             * @param value The value of the entry.
             */
            set(key: Key, value: Value): void;

            /**
             * Retrieves the value associated with the specified key.
             *
             * @param key The key of the entry to retrieve.
             * @return The value associated with the key, or undefined if the key does not exist.
             */
            get(key: Key): Value | undefined;

            /**
             * Checks if the specified key exists in the HashMap.
             *
             * @param key The key to check for existence.
             * @return True if the key exists, false otherwise.
             */
            has(key: Key): boolean;

            /**
             * Deletes the entry associated with the specified key.
             *
             * @param key The key of the entry to delete.
             * @return The value associated with the deleted key, or undefined if the key did not exist.
             */
            delete(key: Key): Value | undefined;

            /**
             * Removes all entries from the HashMap.
             */
            clear(): void;

            /**
             * Executes a provided function once for each key-value pair in the HashMap.
             *
             * @param fn The function to execute for each entry.
             * @param thisArg Optional. Value to use as this when executing fn.
             */
            forEach(fn: (value: Value, key: Key) => void, thisArg?: object): void;

            /**
             * Returns an iterator over the key-value pairs in the HashMap.
             *
             * @return An iterator of [key, value] pairs.
             */
            [Symbol.iterator](): IterableIterator<[Key, Value]>;

            /**
             * Returns an iterable of key-value pairs in the HashMap.
             *
             * @return An iterable iterator of [key, value] pairs.
             */
            entries(): IterableIterator<[Key, Value]>;

            /**
             * Returns an iterable of keys in the HashMap.
             *
             * @return An iterable iterator of keys.
             */
            keys(): IterableIterator<Key>;

            /**
             * Returns an iterable of values in the HashMap.
             *
             * @return An iterable iterator of values.
             */
            values(): IterableIterator<Value>;

            /**
             * The number of entries in the HashMap.
             */
            readonly size: number;
        }
    }
}
