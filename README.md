# TektronixPlugin
Plugin for LabAnalyser to control and visualize data of Tektronix Scopes (using visa).


## How to compile TektronixPlugin (incomplete)

### Prerequisites

The following libraries are necessary:
1. boost_1_59_0 (https://www.boost.org/)
2. LabAnalyser should be located in the same directory as the TektronixPlugin

### Steps  
(tested on Arch Linux)

1. install boost-libs 
   - Arch Linux: `pacman -S boost-libs`
2. install HighFive
3. create build folder `mkdir build`
4.  change folder to build: `cd build`
5.  run qmake inside build: `qmake ../`
6.  run make: `make`
