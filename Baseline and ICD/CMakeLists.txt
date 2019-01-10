cmake_minimum_required(VERSION 3.7)
project(Cuckoo_rev2_5__Final_Traces_)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        Block.cpp
        Block.h
        Cache.cpp
        Cache.h
        CacheBlock.cpp
        CacheBlock.h
        CacheSet.cpp
        CacheSet.h
        CompleteCache.cpp
        CompleteCache.h
        Cuckoo.cpp
        Cuckoo.h
        CuckooBlock.cpp
        CuckooBlock.h
        CuckooWay.cpp
        CuckooWay.h
        main.cpp Coalesce.cpp Coalesce.h)

include_directories(.)

add_executable(Cuckoo_rev2_5__Final_Traces_ ${SOURCE_FILES})