#import <AppKit/AppKit.h>

void HideShortcutWorker() {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyProhibited];
}
