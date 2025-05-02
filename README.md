# Report, C programming for Linux
This will be where i write my report for the C exam.

### Notes for submission
<br />
<br />
<br />
## Task 1

### A) Explain what the C programming language can be used for
The C programming language can theoretically be used to create anything, however it is commonly used in low level systems like embedded systems, operating systems, compilers and other low level software.

### B) Who is Dennis Ritchie and what is he known for in the field of Information Technology?
Dennis Ritchie is mos known for creating the C programming language in the 1970's. He also worked on the UNIX operating system which was used as a foundation of various operating systems we use today.

### C) List at least 5 differnet Linux "distros", explain briefly their background, if they have  any specific purpose, and how they differ from each other.
#### 1. Debian
First announced in 1993, Debian GNU is a free and open sourced distribution. It is mainly used as a desktop operating system by developers and is highly configurable. It is stable and runs of numerous architectures.

#### 2. Ubuntu
First announced in September 2004, Ubuntu is the most popular and widely used linux distribution. It aws created by Mark Shuttleworth, a former Debian developer. It is based on the development version of Debian (called unstable "Sid"), which contributes to the similarity of the distros. It is however not fully compatible with Debian. Its main traits are it accessibility and customizability. 

#### 3. Arch
Created around the year 2002 by Judd Vinet, Arch Linux is an advanced distro made for intermediate and advanced Linux users. Its main feature is its powerful package manager Pacman which can among other things supports the rolling release of the distro, allowing the system to update itself through "rolling releases".
It is created to be configurable to the minutest details, allowing advanced users to mold the distro to their liking. It differs from the previous two with its "rolling releases" which is different from the stable builds of Debian and Ubuntu, and through its catering to advanced users. 

#### 4. FreeBSD
FreeBSD was created in 1993 and comes with over 20 000 packages. It is mainly used for enterprise platforms in the shape of internet/intranet servers and embedded platforms, and is widely regarded for its stability, performance and security. It differs from the other distributions for mainly catering to enterprise platforms.

#### 5. Fedora
Unveiled in September 2004, Fedora is a highly innovative distro which caters to Linux desktop users. It is similar to Debian and Ubuntu in this regard and is one of the most widely used distributions. The developers of this distro focus heavily on innovation and aims to be on the cutting edge of the Linux platform. 
<br />
<br />
<br />
## Task 2

### Initial Setup
I started by creating the source and header file for my main program "task2_main.c", followed by including the makefile and modifying it slighly (see comments). Then i created header files for all the functions EWA created.

#### An aside on header inclusion for EWA generated files...
I ran into an error where <stdio.h> was not included in the function and attempted to include this in the assorted header files. This did not work, therefore i included <stdio.h> in the provided functions, slightly modifying the source files EWA created.

After testing my program i found out that whenever the program is tested, the source files are changed, removing the stdio header and making the program not compile again. This also deleted the source files reference to its accompanying header files, causing me a massive headache. As i have to assume that my program will be ran again during evaluation, i have to figure out a method to link each source file with its header before runtime. While hacky, i can echo the headers into each file before it compiles.

This is implemented in the Makefile. While i recognize that this method may be subversive to the intent of the task, and has not been directly covered in the course material, i see no other reasonable way to do this except including the source files directly in my C program, which would make the header files completely redundant and is bad practice and therefore could cost me points.

Below is the bash script i created through research and some minor problem solving. It checks each of EWA's function files for whether or not it has any inclusions "#". If not, it creates the link to the sources relevant header file. I also added the echo command to signal when it executes in the terminal for the evaluators to see. :)

```
@if [ "$$(head -c 1 task2_count.c)" != "#" ]; then \
	echo "Header file not included in task2_count.c. Adding ..."; \
	sed -i '1i#include "task2_count.h"' task2_count.c; \
fi
@if [ "$$(head -c 1 task2_sum.c)" != "#" ]; then \
	echo "Header file not included in task2_sum.c. Adding ..."; \
	sed -i '1i#include "task2_sum.h"' task2_sum.c; \
fi
@if [ "$$(head -c 1 task2_hash.c)" != "#" ]; then \
	echo "Header file not included in task2_hash.c. Adding ..."; \
	sed -i '1i#include "task2_hash.h"' task2_hash.c; \
fi
```
<br/>
### Creating the program
Implementation of the program was really simple. I used the functions i was given to inject the metadata into my struct and write that struct to a bin file.
<br/>
<br/>
<br/>
## Task 3

### Initial setup
For this program I found that i had a lot of data structures prepared. I had already created the menu structure, single linked list and doubly linked list beforehand, as well as some other utility files (debug and util). However, when i put them all togheter (specifically the linked lists) i found that they had conflicting data types. Initially i tried renaming the wrappers but found the same issue with the naming of methods and the NODE struct. I then decided to refactor these two data structures into FLIGHT_LIST and PASSENGER_LIST, since it prevent unnecessary abstraction. 

### Linked List implementation

### Menu implementation



### Other files

### Creating the final program

## Sources 
See "Sources" document on google docs.
