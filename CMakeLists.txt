cmake_minimum_required(VERSION 3.25)
project(mycb)

find_package(LLVM REQUIRED CONFIG)

add_executable(mycb mycb.cc indexer.cc tuindexer.cc visitor.cc)
target_link_libraries(mycb PRIVATE LLVM)
target_link_libraries(mycb PRIVATE clang-cpp)

# add_executable(test test.cc tuindexer.cc)
# target_link_libraries(test PRIVATE LLVM)
# target_link_libraries(test PRIVATE clang-cpp)
