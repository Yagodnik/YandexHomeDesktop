#import "MacOSIntegration.h"
#import <Cocoa/Cocoa.h>

namespace MacOSIntegration {

void HideAppFromTheDock() {
  [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

}