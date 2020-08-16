# Update cmake
sudo apt upgrade cmake

# Install llvm
sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
sudo apt install llvm
sudo apt install clang

# Install gtest
sudo apt install build-essential

## Compile gtest (optional)
sudo apt-get install cmake libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib

# Install antlr
sudo apt install cmake
sudo apt install uuid-dev
sudo apt install pkg-config
mkdir build && mkdir run && cd build
sudo apt install unzip

## Download antlr for linux c++ runtime from https://www.antlr.org/download.html
cd <some antlr folder>
wget https://www.antlr.org/download/antlr4-cpp-runtime-4.8-source.zip
unzip antlr4-cpp-runtime-4.8-source.zip
mkdir build && mkdir run && cd build
cmake ..
DESTDIR=../run make install
cd ../run/usr/local/include
sudo cp -r antlr4-runtime /usr/local/include
cd ../lib
sudo cp * /usr/local/lib
sudo ldconfig

# Get the antlr jar and set path in CMakeLists.txt
cd /home/<user>/.local/lib
wget https://www.antlr.org/download/antlr-4.8-complete.jar

# Make:
cmake CMakeLists.txt
make

# Adapt your antlr.jar / llvm / antlr path in CMakeLists.txt

# Make llvm (optional)
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -G ../llvm
make



# Windows

## VS

Antlr:  Visual Studio 2013/2015
LLVM:	Visual Studio 2017 or higher

https://visualstudio.microsoft.com/

Community 2019

## Antlr
Download https://www.antlr.org/download/antlr-4.8-complete.jar.
Add antlr4-complete.jar to CLASSPATH, either:
Permanently: Using System Properties dialog > Environment variables > Create or append to CLASSPATH variable
Temporarily, at command line:
SET CLASSPATH=.;C:\Javalib\antlr4-complete.jar;%CLASSPATH%
Create batch commands for ANTLR Tool, TestRig in dir in PATH
 antlr4.bat: java org.antlr.v4.Tool %*
 grun.bat:   java org.antlr.v4.gui.TestRig %*
 
Install git for windows
Install Java
 
## LLVM

https://cmake.org/download/
Python >= 2.7
http://gnuwin32.sourceforge.net/
Do not install the LLVM directory tree into a path containing spaces (e.g. C:\Documents and Settings\...) as the configure step will fail.

cd <where-you-want-llvm-to-live>
git clone https://github.com/llvm/llvm-project.git llvm
cd llvm

open Cmake GUI. Go to llvm/llvm. Configure. Use 64 bit compiler. Generate.

## Gtest

https://docs.microsoft.com/en-us/visualstudio/test/how-to-use-google-test-for-cpp?view=vs-2019

Build release!

Download gtest from github. Go to folder. Run cmake . both in googletest and in root. Open vs project and build all.