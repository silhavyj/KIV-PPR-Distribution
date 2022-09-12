# List all test files that make up the final executable (mytest)
add_executable(
    mytest 
    main.cpp
    tests1.cpp ../src/myproject/math.cpp)

# Add the src directory as an include directory,
# so there is no need to specify relative paths
include_directories(../src)

# Link googletest to the final executable
target_link_libraries(mytest gtest)

# Add the test
add_test(mytest test)