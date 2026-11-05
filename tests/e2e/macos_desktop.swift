import AppKit
import CoreGraphics

func fail(_ message: String) -> Never {
    FileHandle.standardError.write(Data((message + "\n").utf8))
    exit(EXIT_FAILURE)
}

switch CommandLine.arguments.dropFirst().first {
case "displays":
    let screens = NSScreen.screens
    let desktopTop = screens.first?.frame.maxY ?? 0
    func bounds(_ frame: NSRect) -> [Int] {
        // Convert AppKit's bottom-left coordinates to Quartz and Qt coordinates.
        [Int(frame.minX), Int(desktopTop - frame.maxY), Int(frame.width), Int(frame.height)]
    }
    let displays = screens.map { screen in
        ["bounds": bounds(screen.frame), "work_area": bounds(screen.visibleFrame),
         "scale": screen.backingScaleFactor] as [String: Any]
    }
    let data = try JSONSerialization.data(withJSONObject: displays)
    print(String(decoding: data, as: UTF8.self))
case "click":
    guard CommandLine.arguments.count == 4,
          let x = Double(CommandLine.arguments[2]), let y = Double(CommandLine.arguments[3]),
          CGPreflightPostEventAccess() else {
        fail("Native clicks require Accessibility permission for the test runner.")
    }
    let point = CGPoint(x: x, y: y)
    for type: CGEventType in [.leftMouseDown, .leftMouseUp] {
        guard let event = CGEvent(mouseEventSource: nil, mouseType: type,
                                  mouseCursorPosition: point, mouseButton: .left) else {
            fail("Could not create the native mouse event.")
        }
        event.post(tap: .cghidEventTap)
    }
default:
    fail("Expected displays or click x y.")
}
