# Copyright: (C) 2013 iCub Facility
# Authors: Paul Fitzpatrick
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

cmake_minimum_required(VERSION 2.8.7)

find_package(YARP REQUIRED)

include_directories(${YARP_INCLUDE_DIRS})

add_executable(add_int_client_v1 add_int_client_v1.cpp)
target_link_libraries(add_int_client_v1 ${YARP_LIBRARIES})

add_executable(add_int_client_v2 add_int_client_v2.cpp)
target_link_libraries(add_int_client_v2 ${YARP_LIBRARIES})

add_executable(add_int_server_v1 add_int_server_v1.cpp)
target_link_libraries(add_int_server_v1 ${YARP_LIBRARIES})

add_executable(talker talker.cpp)
target_link_libraries(talker ${YARP_LIBRARIES})

add_executable(listener_v1 listener_v1.cpp)
target_link_libraries(listener_v1 ${YARP_LIBRARIES})

add_executable(listener_v2 listener_v2.cpp)
target_link_libraries(listener_v2 ${YARP_LIBRARIES})

