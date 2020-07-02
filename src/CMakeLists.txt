cmake_minimum_required(VERSION 2.8)

project(socks5_proxy)

add_subdirectory(libratel)

set(CMAKE_CXX_STANDARD 17)

set(SRC_LIST
  SocksDecoder.cpp
  SocksDecoder.h
  SocksEncoder.cpp
  SocksEncoder.h
  SocksCommon.cpp
  SocksCommon.h
  SocksSession.cpp
  SocksSession.h
  StateMachine.cpp
  StateMachine.h
  InConnTracker.cpp
  InConnTracker.h
  SocksConnection.cpp
  SocksConnection.h
  SocksInterfaces.h
  SocksLoginPassAuthorizer.cpp
  SocksLoginPassAuthorizer.h
  SocksSessionMng.cpp
  SocksSessionMng.h
  EventTcpServer.cpp
  EventTcpServer.h
  EventSocket.cpp
  EventSocket.h
  EventSocketConnected.cpp
  EventSocketConnected.h
  SocksTypes.h
  LoggerAdapter.cpp
  LoggerAdapter.h)

add_library(socks5_lib STATIC
  ${SRC_LIST})

add_subdirectory(third_party/spdlog)

find_package(spdlog)

target_link_libraries(socks5_lib libratel event spdlog)
target_include_directories(socks5_lib PUBLIC libratel)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC socks5_lib)

if (USE_TEST)
  set(TEST_PROJECT_NAME ${PROJECT_NAME}Test)
  set(TEST_SRC_LIST
    Tests/Mocks.h
    Tests/SocksDecoderTest.cpp
    Tests/SocksEncoderTest.cpp
    Tests/SocksSessionTest.cpp
    Tests/StateMachineTest.cpp
    Tests/InConnTracker.cpp
    Tests/main.cpp)

  add_executable(${TEST_PROJECT_NAME} ${TEST_SRC_LIST})

  target_link_libraries(${TEST_PROJECT_NAME} PRIVATE gmock socks5_lib)
  target_include_directories(${TEST_PROJECT_NAME} PRIVATE . libratel)
endif()
