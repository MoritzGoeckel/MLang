# Update cmake
sudo apt upgrade cmake

# Other packages
sudo apt install cmake
sudo apt install uuid-dev
sudo apt install pkg-config
sudo apt install unzip

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

# Make llvm (optional)
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
mkdir build
cd build
cmake -G ../llvm
make
