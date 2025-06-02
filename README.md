
# What's G4

G4 is the code name for the game I'm working on, inspired by Notch's [0x10c](http://en.wikipedia.org/wiki/0x10c).

One of the key components of the game is the existence of in-game computers that the players will be able to program to help them out with daily activities or maybe even play little games.
Since not all players will have the will or perhaps the skill to code their own things, a default Operating System is provided that will be able to perform all basic activities.

The OS is open source, providing an example of a complex application for the virtual computer. The OS also serves as a proof of concept as development progresses, and is in constant evolution.

The computer's architecture is named APCPU-32, unless I come up with a better name later.

**Note**: I can only work on this on my free time, so progress is slow, with occasional bursts.

# IMPORTANT NOTES

* I'm working on the documentation, and I'll make it available in the next few days.
* **You need to download both this repository (clone it), and the latest binaries available in the Downloads section**
    * The DevKit binaries available in the downloads section. Download only the most recent DevKitBinaries_bxxxx.zip file.
    * Detailed instructions bellow in the **How to get it up and running** section
* You need to have VS 2013 runtime binaries installed. Install both x64 and x86 runtimes from  [HERE](http://www.microsoft.com/en-gb/download/details.aspx?id=40784)
* In order to make the documentation public (The Wiki), I had to make it editable too, since Bitbucket doesn't allow readonly wikis. So **please do not edit the wiki**. If you would like a particular piece of the documentation to be prioritized, let me know.
* **Any questions, leave me a message**. Although there is over 2 years worth of part-time work on this, this repository was put together in a rush so I could share it before the game.
	* You can contact me at ruimvfigueira@gmail.com

# Some quick links

* [Short video of the DevKit in action](https://www.youtube.com/watch?v=cIyHgQvfETE)
* [An original Birthday card I was surprised with, running on the emulator](https://www.youtube.com/watch?v=mbefNs7u3zY) 
* [LiveCoding streaming sessions](https://www.livecoding.tv/fumanchu)
* [TwitchTV Coding sessions](http://www.twitch.tv/myfumanchu)
* [My humble website](http://www.crazygaze.com)

# Virtual Computer Features

* 32-bits CPU with 16 global registers, and FPU (floating point operations)
	* Available CPU speed and amount of RAM per virtual computer not decided yet. Will depend on server hosting costs.
* Instruction set loosely inspired by ARM, with some other sources of inspiration mixed in, trying to provide a balance between ease of use and efficiency on the hosting side
* Allows connecting up to 128 devices to the computer. Default devices installed on every computer are:
	* CPU  - The cpu itself is seen as a device
	* Clock/Timers
	* Screen - Text mode, 80x25, 16 colours. VGA style
	* Keyboard
	* NetworkCard - Allows communicating with other computers, and doubles up as a debug port when using the DevKit
	* DiskController - Allows attaching up to 4 disks
* Other devices will come in the future as development progresses, and technical specifications will be available for whoever wishes to code anything using them

## Advanced features for the nerds

When booting, the computer assumes easy to use defaults, so players can easily code something small in assembly if they wish.
Advanced features are hidden by default. Those features allow the creation of proper operating systems. Some of those features:

* User / Supervisor mode
* Memory protection, allowing running several processes.
* Capable of preemptive multitasking
* Fast context switching
* Several interrupts available (IRQ, Divide by zero, Access violation, Illegal instruction, System Call, etc)
* Devices that use memory mapping allow changing the mapping, instead of using fixed locations
 
## The instruction set, and Devices specifications

Architecture documentation can be found in the wiki, although it's still work in progress.

# What's in this repository

* The Operating System source code. **THIS IS COMPLEX**
* Miscellaneous samples showing small scale coding
	* A minimal "hello world" example provided, entirely in assembly.

# What's NOT in this repository

* DevKit binaries. Those are available as a separate download, as I want to keep binaries out of the repository as much as possible.

# How to get it up and running

## WINDOWS

1. Clone this repository
2. Download the DevKitBinaries, and unpack the zip to the repository root folder, overwriting the contents of the DevKit folder.
3. Double click <Root>/DevKit/bin/run_all.bat
	* This will launch the two required processes with default parameters
		* The Server (HostingServer.exe)
		* The IDE/Simulator (APCPUSim.exe)
4. **The Server is at the moment using 4 hardcoded ports (28000...28004)**
5. The IDE should show 3 windows
	* APCPU IDE - Main window, where you can manage the project, code, debug, etc
	* APCPU Simulator - Where you can interact with the virtual computer
		* Double click the tree on the left to open interaction windows for the several devices (where applicable)
	* Server Manager
		* Here you can create new virtual machines, delete existing ones, and open simulators windows for those virtual machines.

## LINUX

REQUIREMENTS:
Since the Devkit is (not yet) cross-platform, you need [Wine](https://www.winehq.org/)

1. Clone this repository
2. Download the DevKitBinaries, and unpack the zip to the repository root folder, overwriting the contents of the DevKit folder.
3. To launch the server (HostingServer.exe) : ``wine ~/.wine/drive_c/\[DevkitDir\]/bin/HostingServer.exe`` in the cli
4. To launch the IDE/Simulator (APCPUSim.exe) : ``wine ~/.wine/drive_c/\[DevkitDir\]/bin/APCPUSim.exe``  in the cli
5. **The Server is at the moment using 4 hardcoded ports (28000...28004)**
6. The IDE should show 3 windows
	* APCPU IDE - Main window, where you can manage the project, code, debug, etc
	* APCPU Simulator - Where you can interact with the virtual computer
		* Double click the tree on the left to open interaction windows for the several devices (where applicable)
	* Server Manager
		* Here you can create new virtual machines, delete existing ones, and open simulators windows for those virtual machines.

# Operating System status

The OS is currently being developed as a proof of concept for the virtual computer architecture, and will evolve and change as the virtual computer specifications change to better suit the gameplay.
It already has some features present in modern Operating Systems, such as preemptive multitasking and memory protection.
What is in the OS at the moment:

## What's in and working / kind of working

* Preemptive kernel
* Multiple processes allowed, with several threads per process.
* Message queues allowing the kernel to send messages to the processes/threads
* The booting process initializes the kernel, and launches some predefined test processes. The blue status bar indicates what process has focus, its cpu usage, and system wide cpu usage. The existing processes are:
	* "idle" - Process that grabs any unused cpu cycles and halts the cpu until an IRQ happens. It allows calculating the cpu usage.
	* "sysstats" - Shows miscellaneous system stats, and doubles up as an crude explorer, allowing changing between applications. This is also the application showing updating the status bar. It has a couple of keyboard shortcuts to test things
		* **CTRL + SHIFT + S** : Give focus to the sysstats application and switch between what stats are displayed (cpu usage, memory, kernel memory, etc)
		* **BACKSPACE** : If the sysstats has focus, it will return focus to the previous process
		* **CTRL+SHIFT+N** : Switch the next application
		* **CTRL+SHIFT+P** : Switch the previous application
	* "testapp1...5" - Sample applications that don't do anything other than loop.
* Every process that desires to write to the screen has it's own screen buffer, and the kernel will map the focused process's buffer to the Screen Device.
* Both the OS and sample applications are linked in one single binary, but the kernel and application still have their own memory areas
	* This still allows effective memory protection, as the kernel launches a couple of processes
	* This is required at the moment, otherwise the OS itself would need code to load other processes from disk
	* As the OS evolves, it might allow loading processes from a disk once there is enough support for that (as-in, code in the OS to load and resolve symbols, etc)

## Work in progress or not tested

* Drivers for all the default devices exist, although some things are not tested
* The network card allows communicating with other virtual machines, although I didn't test that yet in the VM, and there is no code in the OS for that. The network card at the moment is only used as a debug port to send text logs to the simulator.
* The Disk controller driver is there, but is not doing anything at the moment. There is code included for a file system [FatFS](http://elm-chan.org/fsw/ff/00index_e.html), but I haven't integrated it yet into the OS. Also, since I didn't finish the FatFS integration, the Disk Controller device is the less tested device, and as such there are probably some bugs in there.
* The Screen device pretty much works like a text mode VGA. It allows blinking characters, but that's not support by the simulator yet.

# DevKit

Still work in progress, although it is allowing the creation of the OS. So in my point of view, it's usable as-is, once you know the quirks.
I'll have to create some proper documentation eventually.

## Features

* Project management
* Code completion
* Code navigation
* Very simple debugger
	* You can set breakpoints in C/Assembly files, and inspect the registers, but it doesn't allow watching variables.
* Parallel compilation. On my machine it takes 1-2 seconds to rebuild the entire Operating System. Results may vary :)
* Command console that allows some things not yet available with the UI
	* Type 'help' and it will show a list of commands
	* In short, at the moment it allows some degree of:
		* Memory inspection while debugging,
		* Basic profilling. It allows enabling profiling, collect a snapshot, and log results.
		*  Allows checking function sizes, to decrease code size

## Peculiar things you NEED to pay attention to

* A workspace can have several projects. If it has more than one executable, you can specify the one to launch (There is a dropdown box named "Startup")
* You need to explicitly set what file is to be used as a boot file, as the linker needs to know that file needs to be linked first. This is necessary as the boot file should contain something required by the architecture. You can set the boot file by right clicking an ASM file and picking "Boot File" from the context menu. See the provided Hello World" sample for an explanation of what the boot file needs.

## Available shortcuts

They mostly emulate Visual Studio and/or Visual Assist shortcuts

* **F7** - Build workspace. This should compile only what changed, but not sure it's working properly at the moment. Since it only takes 1-2 seconds to build the entire OS on my machine, I always do a full rebuild
* **CTRL + ALT + F7** - Full rebuild
* **CTRL + F7** - Compile only the current file
* **F5** - Starts the program. **Once running it, it won't let you edit code unless you detach the debugger. Click the big red "STOP" button on the top bar.**
* **CTRL + F5** - Starts the program without the debugger
* **CTRL + S** - Save your bacon. **SAVE YOUR BACON!**
* **CTRL + F** - Find in current file
* **CTRL + SHIFT + F** - Find in workspace
* **SHIFT + ALT + O** - Go to file. Start typing and it will filter
* **SHIFT + ALT + S** - Find symbol. Start typing and it will filter
* **F12** - Goto definition
* **CTRL + ALT + F12** - Goto declaration
* **ALT + O** - Switch between .H and .C file
* **ALT + M** - Find symbol in file. Start typing and it will filter. Good for navigating within a single file
* **CTRL + SHIFT + G** - Goto the file in the #include statement under the cursor.
* **CTRL + '-'** - Goto to previous cursor position. **WORK IN PROGRESS**
* **CTRL + SHIFT +  '-'** - Goto to next cursor position. **WORK IN PROGRESS**
* **CTRL + SPACE** - Explicitly show auto complete dialog if applicable
* **CTRL + SHIFT + SPACE** - Explicitly show the call tip for the current function call you 
are typing
* **CTRL + SHIFT + R** - Do a full reparse of the workspace, to refresh the code completion/navigation database and syntax colorizing.

## Known problems

* Code completion sometimes causes the IDE to crash. **SAVE OFTEN**. This mostly happens when typing "." inside comments, as it will try code completion and trigger what it seems to be a multithreaded related bug in libclang.
* Setting/Removing breakpoints while the process is already in the "break" state will sometimes cause it to never go past that instruction. Stop the debugging and remove all breakpoints.
* The several tree controls used, such as in the Project window, Devices in the simulator, and the Server Manager will often automatically refresh and cause some annoying things when you try to click them.
	* The Server Manager allows disabling the automatic refresh, and provides a button for explicit refresh
* The C compiler is a port from [VBCC](http://www.compilers.de/vbcc.html) . It's quite usable, but the port still has bugs which causes it generate invalid code in some situations. I'm fixing whatever I find as I develop the OS.

# Disclaimers

* **0x10c** is a registered trademark of Mojang AB

