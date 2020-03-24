option(ENABLE_NQ_SIGNALS "Enables build and linking NQ signals" ON)
option(USE_TEST "Enables testing" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

if (ENABLE_NQ_SIGNALS)
  set(NQ_SRC_LIST
    NQSignals/Private/Wrapper.h
    NQSignals/Private/SignalHash.h
    NQSignals/Private/SignalCallback.h
    NQSignals/Private/SignalConnectionStorage.cpp
    NQSignals/Private/SignalConnectionStorage.h
    NQSignals/Private/SignalStorage.h

    NQSignals/Private/SignalHandler.cpp
    NQSignals/Private/SignalHandler.h
    NQSignals/NQObject.cpp
    NQSignals/NQObject.h
    NQSignals/NQCoreApplication.cpp
    NQSignals/NQCoreApplication.h
  )
endif()

add_library(ratel-lib ${LIBTYPE}
  InetUtils.cpp
  InetUtils.h
  MemReader.cpp
  MemReader.h
  MemWriter.cpp
  MemWriter.h
  Common.h
  Optional.h
  ExceptionStream.h
  Serialization.cpp
  Serialization.h
  TcpSocket.cpp
  TcpSocket.h
  TcpServer.cpp
  TcpServer.h
  Event2Server.cpp
  Event2Server.h
  ConcurrentQueue.h
  StringUtils.cpp
  StringUtils.h

  ${NQ_SRC_LIST}
)

if (ENABLE_NQ_SIGNALS)
  target_include_directories(ratel-lib PUBLIC . NQSignals)
else()
  target_include_directories(ratel-lib PUBLIC .)
endif()

set_property(TARGET ratel-lib PROPERTY CXX_STANDARD 17)

if (WIN32)
  target_link_libraries(ratel-lib Ws2_32)
endif()
target_link_libraries(ratel-lib event)

if (USE_TEST)
  set(BUILD_GMOCK TRUE)
  add_subdirectory(testing)
endif()

add_subdirectory(tracer/src)
target_link_libraries(ratel-lib libtracer)
