<?php
// Copyright (c) ZeroC, Inc.

namespace Ice;

class Value
{
    public $_ice_slicedData;

    public static function ice_staticId()
    {
        return "::Ice::Object";
    }

    public function ice_id()
    {
        return "::Ice::Object";
    }

    public function ice_getSlicedData()
    {
        return $this->_ice_slicedData;
    }
}

global $Ice__t_Value;
$Ice__t_Value = IcePHP_defineClass('::Ice::Object', "\\Ice\\Value", -1, null, null);

class UnknownSlicedValue extends Value
{
    public $unknownTypeId;

    public function __construct()
    {
    }

    public function ice_id()
    {
        return $this->unknownTypeId;
    }
}

global $Ice__t_UnknownSlicedValue;
$Ice__t_UnknownSlicedValue = IcePHP_defineClass('::Ice::UnknownSlicedValue', "\\Ice\\UnknownSlicedValue", -1,
                                                $Ice__t_Value, null);

class SlicedData
{
    public $slices;
}

class SliceInfo
{
    public $typeId;
    public $compactId;
    public $bytes;
    public $instances;
    public $hasOptionalMembers;
    public $isLastSlice;
}
