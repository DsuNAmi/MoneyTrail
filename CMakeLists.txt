cmake_minimum_required(VERSION 3.10)
project(MoneyTrail)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_ROOT "/opt/homebrew")
find_package(Boost 1.70.0 REQUIRED COMPONENTS system thread)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INLCUDE_DIRS}
    ${Nlohmann_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/includes
)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/backend/*.cpp"
)

add_executable(money_trail ${SERVER_SOURCES})

target_link_libraries(
    money_trail
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
    ${Nlohmann_LIBRARIES}
    pthread
)

