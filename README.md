# G4 DevkKit#

### What's G4 ###

G4 is the code name for the game I'm working on, inspired by Notch's [0x10c](http://en.wikipedia.org/wiki/0x10c).

One of the key components of the game is the existence of in-game computers that the players will be able to program to help them out with daily activities or maybe even play little games.
Since not all players will have the will or perhaps the skill to code their own things, a default Operating System is provided that will be able to perform all basic activities.

The OS is open source, providing an example of a complex application for the virtual computer. The OS also serves as a proof of concept as development progresses, and is in constant evolution.

The computer's architecture is named APCPU-32, unless I come up with a better name later.

### Virtual Computer Basic Features ###

* 32-bits CPU with 16 global registers, and FPU (floating point operations)
* Instruction set loosely inspired by ARM, with some other sources of inspiration mixed in, trying to provide a balance between ease of use and efficiency on the hosting side
+ Allows connecting up to 128 devices to the computer. Default devices installed on every computer are:
	* CPU  - The cpu itself is seen as a device)
	* Clock/Timers
	* Screen - Text mode, 80x25, 16 colours)
	* Keyboard
	* NetworkCard - Allows communicating with other computers, and doubles up as a debug port when using the DevKit
	* DiskController - Allows attaching up to 4 disks
- Other devices will come in the future as development progresses, and technical specifications will be available for whoever wishes to code anything using them

### Advanced features for the nerds ###

When booting, the computer assumes easy to use defaults, so players can easily code something small in assembly if they wish.
Advanced features are but hidden by default. Those features allow the creation of proper operating systems. Some of those features:

* User / Supervisor mode
* Memory protection, allowing running several processes.
* Capable of preemptive multitasking
* Fast context switching
* Several interrupts available (IRQ, Divide by zero, Access violation, Illegal instruction, System Call, etc)
* Devices that use memory mapping allow changing the mapping, instead of using fixed locations
 
### What's in this repository ###

* The Operating System source code
* Miscellaneous samples showing how to code for the virtual machine

### What's NOT in this repository ###

* DevKit binaries. Those are available as a separate download

### How to get it up and running ###

1. Clone this repository
2. Download the DevKitBinaries, and unpack the zip to the repository root folder, overwriting the contents of the DevKit folder.
3. Double click <Root>/DevKit/bin/run_all.bat
	* This will launch the two required processes with default parameters
		* The Server (HostingServer.exe)
		* The IDE/Simulator (APCPUSim.exe)
4. **The Server is at the moment using 4 hardcoded ports (28000...28005)**
5. The IDE should show 3 windows
	* APCPU IDE - Main window, where you can manage the project, code, debug, etc
	* APCPU Simulator - Where you can interact with the virtual computer
		* Double click the tree on the left to open interaction windows for the several devices (where applicable)
	* Server Manager
		* Here you can create new virtual machines, delete existing ones, and open simulators windows for those virtual machines.

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact