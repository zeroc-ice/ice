//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import UIKit
import Darwin

class ViewController: UIViewController, UIPickerViewDataSource, UIPickerViewDelegate {

    @IBOutlet weak var interfaceIPv4: UIPickerView!
    @IBOutlet weak var interfaceIPv6: UIPickerView!
    @IBOutlet weak var output: UITextView!

    var interfacesIPv4: [String]!
    var interfacesIPv6: [String]!

    override func viewDidLoad() {
        super.viewDidLoad()

        interfacesIPv4 = [String]()
        interfacesIPv4.append("127.0.0.1")

        interfacesIPv6 = [String]()
        interfacesIPv6.append("::1")

        var ifap: UnsafeMutablePointer<ifaddrs>?

        if getifaddrs(&ifap) == 0 {
            for curr in sequence(first: ifap!, next: { $0.pointee.ifa_next }) {
                if (curr.pointee.ifa_flags & UInt32(IFF_UP)) != 0 &&
                    (curr.pointee.ifa_flags & UInt32(IFF_LOOPBACK)) == 0 {

                    if curr.pointee.ifa_addr!.pointee.sa_family == UInt32(AF_INET) {
                        var buf = [Int8](repeating: 0, count: Int(INET_ADDRSTRLEN))
                        curr.pointee.ifa_addr!.withMemoryRebound(to: sockaddr_in.self, capacity: 1) { addr in
                            let s = inet_ntop(AF_INET, &addr.pointee.sin_addr, &buf, socklen_t(INET_ADDRSTRLEN))
                            interfacesIPv4.append("\(String(cString: s!))")
                        }
                    } else if curr.pointee.ifa_addr!.pointee.sa_family == UInt32(AF_INET6) {
                        var buf = [Int8](repeating: 0, count: Int(INET6_ADDRSTRLEN))
                        curr.pointee.ifa_addr!.withMemoryRebound(to: sockaddr_in6.self, capacity: 1) { addr6 in
                            let s = inet_ntop(AF_INET6, &addr6.pointee.sin6_addr, &buf, socklen_t(INET6_ADDRSTRLEN))
                            interfacesIPv6.append("\(String(cString: s!))")
                        }
                    }
                }
            }
            freeifaddrs(ifap)
        }

        // By default, use the loopback
        interfaceIPv4.selectedRow(inComponent: 0)
        interfaceIPv6.selectedRow(inComponent: 0)

        do {
            try ControllerI.startController(view: self, ipv4: interfacesIPv4[0], ipv6: interfacesIPv6[0])
        } catch {
            println("Error: \(error)")
        }
    }

    public func write(_ msg: String) {
        output.insertText(msg)
        output.layoutIfNeeded()
        output.scrollRangeToVisible(NSRange(location: output.text.count - 1, length: 1))
    }

    public func print(_ msg: String) {
        DispatchQueue.main.async {
            self.write(msg)
        }
    }

    public func println(_ msg: String) {
        print("\(msg)\n")
    }

    func numberOfComponents(in pickerView: UIPickerView) -> Int {
        return 1
    }

    func pickerView(_ view: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        if view === interfaceIPv4 {
            return interfacesIPv4.count
        } else {
            return interfacesIPv6.count
        }
    }

    func pickerView(_ view: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        if view === interfaceIPv4 {
            return interfacesIPv4[row]
        } else {
            return interfacesIPv6[row]
        }
    }

    func pickerView(_ pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        do {
            ControllerI.stopController()
            try ControllerI.startController(view: self,
                                            ipv4: interfacesIPv4[interfaceIPv4.selectedRow(inComponent: 0)],
                                            ipv6: interfacesIPv6[interfaceIPv6.selectedRow(inComponent: 0)])
        } catch {
            print("Error: \(error)")
        }
    }
}
