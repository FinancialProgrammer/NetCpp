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

add_executable(yourApp ...)

target_link_libraries(yourApp netcpp)

```
or
```bash
g++ yourprogram.cpp -lnetcpp
```


## Documentation
### Socket Class
Default socket which should only be used for internal communciations between programs.

### SocketSSL
Built from the socket class in pair with Open SSL <br>
Uses SSL/TLS Encryption to support communication with https sockets <br>

