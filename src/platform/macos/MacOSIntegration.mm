#import "MacOSIntegration.h"
#import <Cocoa/Cocoa.h>
#include "../NotchActivation.h"

namespace MacOSIntegration {

QRect NotchUnderCursor() {
    if (@available(macOS 12.0, *)) {
        const NSPoint mouse = [NSEvent mouseLocation];
        const CGFloat primaryTop = NSMaxY([NSScreen screens].firstObject.frame);
        for (NSScreen *screen in [NSScreen screens]) {
            if (screen.safeAreaInsets.top <= 0 || !NSPointInRect(mouse, screen.frame)) continue;
            const NSRect left = screen.auxiliaryTopLeftArea;
            const NSRect right = screen.auxiliaryTopRightArea;
            const NSRect notch = NSMakeRect(NSMaxX(left), NSMaxY(screen.frame) - screen.safeAreaInsets.top,
                                            NSMinX(right) - NSMaxX(left), screen.safeAreaInsets.top);
            const QRectF anchor(notch.origin.x, primaryTop - NSMaxY(notch),
                                notch.size.width, notch.size.height);
            const QPointF cursor(mouse.x, primaryTop - mouse.y);
            if (IsNotchActivationPoint(anchor, cursor))
                return anchor.toAlignedRect();
        }
    }
    return {};
}

void HideAppFromTheDock() {
    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

void ShowAppFromTheDock() {
    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
}

}