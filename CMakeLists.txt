cmake_minimum_required(VERSION 3.16)

project(bytesqueeze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bytesqueeze main.cpp
    RLE.cpp
    RLE.h
    LZ78.h
    LZ78.cpp
    Encryption.cpp
    Encryption.h
    Serialization.cpp
    Serialization.h)

include(GNUInstallDirs)
install(TARGETS bytesqueeze
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
