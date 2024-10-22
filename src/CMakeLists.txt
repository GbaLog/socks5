cmake_minimum_required(VERSION 2.8)

project(socks5_proxy)

add_subdirectory(libratel)

set(CMAKE_CXX_STANDARD 17)

set(SRC_LIST
  Event/EventTcpServer.cpp
  Event/EventTcpServer.h
  Event/EventSocket.cpp
  Event/EventSocket.h
  Event/EventSocketConnected.cpp
  Event/EventSocketConnected.h
  Event/EventSocketCommon.cpp
  Event/EventSocketCommon.h
  Event/EventBaseObject.cpp
  Event/EventBaseObject.h
  Event/EventSignalListener.cpp
  Event/EventSignalListener.h

  SocksDecoder.cpp
  SocksDecoder.h
  SocksEncoder.cpp
  SocksEncoder.h
  SocksCommon.cpp
  SocksCommon.h
  StateMachine.cpp
  StateMachine.h
  InConnTracker.cpp
  InConnTracker.h
  DirectedProxyConnection.cpp
  DirectedProxyConnection.h
  Session.cpp
  Session.h
  SessionMng.cpp
  SessionMng.h
  TcpStreamProxy.cpp
  TcpStreamProxy.h
  SocksInterfaces.h
  SocksLoginPassAuthorizer.cpp
  SocksLoginPassAuthorizer.h
  SocksTypes.h
  LoggerAdapter.cpp
  LoggerAdapter.h)

add_library(socks5_lib STATIC
  ${SRC_LIST})

add_subdirectory(third_party/spdlog)

find_package(spdlog)

target_link_libraries(socks5_lib libratel event spdlog)
target_include_directories(socks5_lib PUBLIC . libratel Event)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC socks5_lib)

if (USE_TEST)
  set(TEST_PROJECT_NAME ${PROJECT_NAME}Test)
  set(TEST_SRC_LIST
    Tests/Mocks.h
    Tests/SocksDecoderTest.cpp
    Tests/SocksEncoderTest.cpp
    Tests/StateMachineTest.cpp
    Tests/InConnTrackerTest.cpp
    Tests/main.cpp)

  add_executable(${TEST_PROJECT_NAME} ${TEST_SRC_LIST})

  target_link_libraries(${TEST_PROJECT_NAME} PRIVATE gmock socks5_lib)
  target_include_directories(${TEST_PROJECT_NAME} PRIVATE . libratel)
endif()
