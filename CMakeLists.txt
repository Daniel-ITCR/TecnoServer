cmake_minimum_required(VERSION 3.10)
project(TecnoServer)

set(CMAKE_CXX_STANDARD 11)

include_directories(ChSocket)

find_package(Threads)

add_compile_options("-L/lib -ljson-c")
link_libraries("-L/lib -ljson-c")

add_executable(TecnoServer
        ChSocket/Socket.c
        ChSocket/Socket.h
        ChSocket/Socket_Cliente.c
        ChSocket/Socket_Cliente.h
        ChSocket/Socket_Servidor.c
        ChSocket/Socket_Servidor.h

        CMakeLists.txt
        Constants.c
        Constants.h
        DataManager.c
        DataManager.h
        ChSocket/Communication.h
        ChSocket/Communication.c
        main.c)

target_link_libraries(TecnoServer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(TecnoServer m)
