# NetCpp
A C++ Networking library

## Install
#### CMake
Simple Install using...
```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel
sudo cmake --install .
```

Then Link using...
```cmake

find_library(netcpp REQUIRED)

add_exectuable(yourApp ...)

target_link_libraries(yourApp netcpp)

```
or
```bash
g++ yourprogram.cpp -lnetcpp
```


## Documentation
### Socket Class
Allows an encryption wrapper <br>

### Http Socket
Built from the socket class in pair with Open SSL <br>
Uses SSL/TLS Encryption to support communication with http sockets <br>
