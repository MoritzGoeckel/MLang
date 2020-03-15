# Install antlr

sudo apt install cmake
sudo apt install uuid-dev
sudo apt install pkg-config
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

# Make:
cmake CMakeLists.txt
make
