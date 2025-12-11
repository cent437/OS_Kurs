cmake_minimum_required(VERSION 3.5...4.0)
project(OS_Kurs C CXX)


#add_executable(client src/sender.c)
add_executable(server src/main.c src/receiver.c)
#target_link_libraries(client wsock32)
target_link_libraries(server wsock32)