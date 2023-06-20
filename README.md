# ARP_Assign_02

[Hafiz Muizz Ahmed Sethi ID: S5428151](https://github.com/notMuizz)<br>
[M.Sc Robotics Engineering](https://corsi.unige.it/corsi/10635)<br>


## Description

### Master
This process manages the creation and termination of two subprocesses. Two pipes are utilized to retrieve the process IDs (PIDs) of the subprocesses, which are necessary for their termination. Additionally, the SIGINT signal is handled to ensure proper closure of both the master and child processes.

### PROCESS A
In this process, shared memory is established, along with the initialization of a semaphore to safeguard it. After creating and closing a pipe to transmit the PID to the master, three functions are employed. The first function used is **delete** responsible for clearing the bitmap.
The second function is **bmp_circle** which draws a circle at the center of the bitmap. The third function, **static_conversion** allows access to the shared memory and transfers the bitmap data into it. This function is protected by a semaphore to prevent the second process from accessing the memory simultaneously.
Subsequently, an infinite while loop begins, wherein pressing the **P** button triggers the capture of a screenshot of the bitmap. If any of the four arrow keys on the keyboard are pressed, the circle on the initial interface is moved and redrawn using the functions **move_circle** and **draw_circle** Finally, the three aforementioned functions are executed in the same order, but the **bmp_circle** function is provided with the coordinates of the interface design.

### PROCESS B
Similar to Process A, a pipe is opened and closed in Process B to transmit the PID to the master.
Before the while loop, the "delete" function (as described above) is utilized. Within the while loop, I have implemented the functionality to capture a screenshot of the bitmap in this process by pressing the **S** key on the keyboard.
the **get_center** function is called, which obtains the coordinates of the circle's center in the bitmap. These coordinates are obtained based on the radius of the circle. Finally, using "mvdacch," zeros are displayed on the screen, representing the movement of the drawing in the initial interface.

## Required libraries/packages
### Install Libbitmap
 
 download from https://github.com/draekko/libbitmap

navigate to the root directory of the downloaded repo and type `./configure`

type `make`

run `make install`

### Install ncurses

```console
sudo apt-get install libncurses-dev
```
## Compiling and running the code
In order to compile all the processes that make up the program, you just need to execute the following command:
```console
chmod +x install.src
./install.src
```

Then, you can run the program by executing:
```console

chmod +x run.src
./run.src
```

## User Interface 

There are two interfaces in this system. In the first interface, a small drawing is displayed, and it can be moved by using the arrow keys on the keyboard. Additionally, clicking on the area with a 'P' will capture a screenshot of the bitmap.

In the second interface, a series of zeros will appear, tracing the movements of the drawing from the first interface. By pressing the "s" key on the keyboard, a screenshot of the bitmap from process B can be obtained.

It's important to note that process A operates within a window size of 90x30, while process B operates within a window size of 80x30.
