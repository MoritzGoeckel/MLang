 sudo apt upgrade cmake

# Install llvm
sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
sudo apt install llvm
sudo apt install clang

# Install gtest
sudo apt install build-essential

## Maybe compile gtest
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

# Delete the runtime folder
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
