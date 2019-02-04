//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        type HashMapKey = number | string | boolean |
            {
                equals(other: any):boolean;
                hashCode():number
            };

        class HashMap<Key extends HashMapKey, Value>
        {
            constructor();
            constructor(other:HashMap<Key, Value>);
            constructor(keyComparator: (k1:Key, k2:Key) => boolean,
                        valueComparator?: (v1:Value, v2:Value) => boolean);

            set(key:Key, value:Value):void;
            get(key:Key):Value|undefined;
            has(key:Key):boolean;
            delete(key:Key):Value|undefined;
            clear():void;
            forEach(fn:(value:Value, key:Key)=>void, thisArg?:Object):void;

            entries(): IterableIterator<[Key, Value]>;
            keys(): IterableIterator<Key>;
            values(): IterableIterator<Value>;

            equals(other: HashMap<Key, Value>, valueComparator?: (v1:Value, v2:Value) => boolean): boolean;
            merge(from: HashMap<Key, Value>):void;

            readonly size:number;
        }
    }
}
