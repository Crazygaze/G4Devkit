
# What's G4

G4 is the code name for a game inspired by Notch's 0x10c.

A core feature of the game is the presence of in-game computers that players can program to help with daily tasks—or even to play small games.

Since not all players will want (or be able) to write their own code, a default operating system is provided. It can handle all the basic activities out of the box. Development tools for the in-game computer, the source code for the default OS, and some sample programs can be found here: https://github.com/crazygaze/g4devkit

Here's a very old video showing the development tools in action:  https://www.youtube.com/watch?v=cIyHgQvfETE

The default OS also acts as a proof of concept for the virtual machine. It uses many of the more advanced features—even though most players probably won’t need them in their own custom OSes—so it's a bit more complex than strictly necessary.

The computer architecture is currently called APCPU-32, though I might come up with a better name later.

I’ve been working on G4 off and on since 2012. Development paused for a few years while I was at Epic Games, but I've recently picked it back up.

**Note**: I can only work on this on my free time, so progress is slow, with occasional bursts.

# Virtual Computer Features

* 32-bits CPU with 16 global registers, and FPU (floating point operations)
	* Available CPU speed and amount of RAM per virtual computer not decided yet. Will depend on server hosting costs.
* Instruction set loosely inspired by ARM, with some other sources of inspiration mixed in, trying to provide a balance between ease of use and efficiency on the hosting side
* Allows connecting up to 32 devices to the computer. Default devices installed on every computer are:
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
* Fully functional MMU, providing memory protection and address translation (Virtual Memory)
* Capable of preemptive multitasking
* Fast context switching
* Several interrupts available (IRQ, Divide by zero, Access violation, Illegal instruction, System Call, etc)
* Devices that use memory mapping allow changing the mapping, instead of using fixed locations
 

# What's in this repository

* The Operating System source code. **THIS IS COMPLEX**
* Miscellaneous samples showing small scale coding
	* A minimal "hello world" example provided, entirely in assembly.
* At some point I'll open source the toolchain too.
* The DevKit binaries available in the downloads section.
    * Detailed instructions bellow in the **How to get it up and running** section
* Documentation is a work in progress.

# Some quick links

* [Short video of the DevKit in action](https://www.youtube.com/watch?v=cIyHgQvfETE)
* [TwitchTV Coding sessions](http://www.twitch.tv/myfumanchu)
* [My humble website](http://www.crazygaze.com)


# How to get it up and running

** TODO **

# Operating System status

The OS is currently being developed as a proof of concept for the virtual computer architecture, and will evolve and change as the virtual computer specifications change to better suit the gameplay.


