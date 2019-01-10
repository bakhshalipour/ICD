cmake_minimum_required(VERSION 3.7)
project(src)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Block.cpp
        Block.h
        Cache.cpp
        Cache.h
        FWPEntry.cpp
        FWPEntry.h
        FWPSet.cpp
        FWPSet.h
        main.cpp
        SegmentPredictor.cpp
        SegmentPredictor.h
        Set.cpp
        Set.h
        util.h)

include_directories(.)

add_executable(src ${SOURCE_FILES})