//
//  File.swift
//  TestDriver
//
//  Created by jose on 3/1/19.
//

import TestCommon
import IceAcm

class TestFactory {
    create(name: String) -> TestCommon.TestHelper {
    helper = nil
    switch(name) {

    case "Ice.acm.Client":
        helper = IceAcm.Client()
    
    case "Ice.adapterDeactivation.Client":
        helper = IceAcm.Client()
    
    case "Ice.admin.Client":
        helper = IceAdmin.Client()
    
    case "Ice.ami.Client":
        helper = IceAmi.Client
    
    case "Ice.binding.Client":
        helper = IceBinding.Client
    
    case "Ice.defaultServant.Client":
        helper = IceDefaultServant.Client()
    
    case "Ice.defaultValue.Client":
        helper = IceDefaultValue.Client()
    
    case "Ice.dispatcher.Client":
        helper = IceDispatcher.Client()
    
    case "Ice.enums.Client":
        helper = IceEnums.Client()
    
    case "Ice.exceptions.Client":
        helper = IceExceptions.Client()
    
    case "Ice.facets.Client":
        helper = IceFacets.Client()
    
    case "Ice.hold.Client":
        helper = IceHold.Client()
    
    case "Ice.info.Client":
        helper = IceInfo.Client()
    }
    return helper
}
