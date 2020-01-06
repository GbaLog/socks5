option(ENABLE_JK_SIGNALS "Enables build and linking JK signals" ON)

if (ENABLE_JK_SIGNALS)
  set(JK_SRC_LIST
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
else()
  set(GB_SRC_LIST)
endif()

add_library(ratel-lib ${LIBTYPE}
  InetUtils.cpp
  InetUtils.h
  MemReader.cpp
  MemReader.h
  MemWriter.cpp
  MemWriter.h
  Tracer.h
  Tracer.cpp
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

  ${JK_SRC_LIST}
)

if (ENABLE_JK_SIGNALS)
  target_include_directories(ratel-lib PUBLIC . NQSignals)
else()
  target_include_directories(ratel-lib PUBLIC .)
endif()

set_property(TARGET ratel-lib PROPERTY CXX_STANDARD 17)

if (WIN32)
  target_link_libraries(ratel-lib Ws2_32 event)
endif()

if (USE_GTEST)
  set(BUILD_GMOCK TRUE)
  add_subdirectory(testing)
endif()
