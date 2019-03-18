//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Twoways {

    public static func twoways(_ helper: TestHelper, _ p: MyClassPrx) throws {
        let communicator = helper.communicator()
        
        let literals = try p.opStringLiterals()
        
        try test(s0 == "\\" &&
                 s0 == sw0 &&
                 s0 == literals[0] &&
                 s0 == literals[11])
        
        try test(s1 == "A" &&
                 s1 == sw1 &&
                 s1 == literals[1] &&
                 s1 == literals[12])
        
        try test(s2 == "Ice" &&
                 s2 == sw2 &&
                 s2 == literals[2] &&
                 s2 == literals[13])
        
        try test(s3 == "A21" &&
                 s3 == sw3 &&
                 s3 == literals[3] &&
                 s3 == literals[14])
        
        try test(s4 == "\\u0041 \\U00000041" &&
                 s4 == sw4 &&
                 s4 == literals[4] &&
                 s4 == literals[15])
        
        try test(s5 == "\u{00FF}" &&
                 s5 == sw5 &&
                 s5 == literals[5] &&
                 s5 == literals[16])
        
        try test(s6 == "\u{03FF}" &&
                 s6 == sw6 &&
                 s6 == literals[6] &&
                 s6 == literals[17])
        
        try test(s7 == "\u{05F0}" &&
                 s7 == sw7 &&
                 s7 == literals[7] &&
                 s7 == literals[18])
        
        try test(s8 == "\u{10000}" &&
                 s8 == sw8 &&
                 s8 == literals[8] &&
                 s8 == literals[19])
        
        try test(s9 == "\u{1F34C}" &&
                 s9 == sw9 &&
                 s9 == literals[9] &&
                 s9 == literals[20])
        
        try test(s10 == "\u{0DA7}" &&
                 s10 == sw10 &&
                 s10 == literals[10] &&
                 s10 == literals[21])
        
        try test(ss0 == "\'\"\u{003f}\\\u{0007}\u{0008}\u{000c}\n\r\t\u{000b}\u{0006}" &&
                 ss0 == ss1 &&
                 ss0 == ss2 &&
                 ss0 == literals[22] &&
                 ss0 == literals[23] &&
                 ss0 == literals[24])
        
        try test(ss3 == "\\\\U\\u\\" &&
                 ss3 == literals[25])
        
        try test(ss4 == "\\A\\" &&
                 ss4 == literals[26])
        
        try test(ss5 == "\\u0041\\" &&
                 ss5 == literals[27])
        
        try test(su0 == su1 &&
                 su0 == su2 &&
                 su0 == literals[28] &&
                 su0 == literals[29] &&
                 su0 == literals[30])
        
        try p.ice_ping();

    }
    
    public static func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        if !value {
            throw TestFailed.testFailed
        }
    }
}
