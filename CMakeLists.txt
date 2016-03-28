cmake_minimum_required(VERSION 3.4)
include(CheckCXXCompilerFlag)
project(LaSVM CXX C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CHECK_CXX_COMPILER_USED1)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
  # C++11
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX17)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  elseif(COMPILER_SUPPORTS_CXX1Z)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
  elseif(COMPILER_SUPPORTS_CXX14)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif(COMPILER_SUPPORTS_CXX1Y)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  elseif(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif(COMPILER_SUPPORTS_CXX17)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

  if(CHECK_CXX_COMPILER_USED2)
  elseif("${CMAKE_CXX_COMPILER_ID}x" MATCHES "Clangx")
    # using Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-global-constructors")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "GNUx")
    # using GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
  endif(CHECK_CXX_COMPILER_USED2)
endif(CHECK_CXX_COMPILER_USED1)

#Boost
FIND_PACKAGE(Boost)
if(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
else()
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/local/boost_1_60_0/")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/local/boost_1_60_0/lib64-msvc-14.0")
    Find_PACKAGE( Boost )
endif()

#LaSVM_TOOLS
file(GLOB_RECURSE LaSVM_TOOLS_HEADERS
	"${LaSVM_SOURCE_DIR}/src/lasvm/*.h" "${LaSVM_SOURCE_DIR}/src/lasvm/*.hpp"
)

file(GLOB_RECURSE LaSVM_TOOLS_SRC
	"${LaSVM_SOURCE_DIR}/src/lasvm/*.c" "${LaSVM_SOURCE_DIR}/src/lasvm/*.cc"
)

add_library(lasvm ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_TOOLS_SRC})
target_link_libraries (lasvm
		${Boost_LIBRARIES}
	)

if(CHECK_CXX_COMPILER_USED_TOOLS)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (lasvm
		m
	)
endif(CHECK_CXX_COMPILER_USED_TOOLS)

#LaSVM_IN/OUT
file(GLOB_RECURSE LaSVM_IN_OUT_HEADERS
	"${LaSVM_SOURCE_DIR}/src/io/*.hpp"
)

file(GLOB_RECURSE LaSVM_IN_OUT_SRC
	"${LaSVM_SOURCE_DIR}/src/io/*.c*"
)

add_library(io ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_IN_OUT_SRC})
target_link_libraries (io
		lasvm ${Boost_LIBRARIES}
	)

if(CHECK_CXX_COMPILER_USED_TOOLS)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (io
		m
	)
endif(CHECK_CXX_COMPILER_USED_TOOLS)

#LaSVM_TEST
file(GLOB_RECURSE LaSVM_TEST_HEADERS
	"${LaSVM_SOURCE_DIR}/test/*.hpp"
)

file(GLOB_RECURSE LaSVM_TEST_SRC
	"${LaSVM_SOURCE_DIR}/test/*.cc"
)

add_executable(tests ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_TEST_HEADERS} ${LaSVM_TEST_SRC})
target_link_libraries (tests
		lasvm io ${Boost_LIBRARIES}
	)

if(CHECK_CXX_COMPILER_USED_TOOLS)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (tests
		m
	)
endif(CHECK_CXX_COMPILER_USED_TOOLS)

#Binaries
#la_train
file(GLOB_RECURSE LaSVM_la_train_HEADERS
)

file(GLOB_RECURSE LaSVM_la_train_SRC
	"${LaSVM_SOURCE_DIR}/src/run/la_train.cpp"
)

add_executable(la_train ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_la_train_HEADERS} ${LaSVM_la_train_SRC})
target_link_libraries (la_train
	lasvm io
)
target_link_libraries (la_train
		${Boost_LIBRARIES}
	)


if(CHECK_CXX_COMPILER_USED_la_train)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (la_train
		m
	)
endif(CHECK_CXX_COMPILER_USED_la_train)

# la_test
file(GLOB_RECURSE LaSVM_la_test_HEADERS
)

file(GLOB_RECURSE LaSVM_la_test_SRC
	"${LaSVM_SOURCE_DIR}/src/run/la_test.cpp"
)

add_executable(la_test ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_la_test_HEADERS} ${LaSVM_la_test_SRC})
target_link_libraries (la_test
	lasvm io
)
target_link_libraries (la_test
		${Boost_LIBRARIES}
	)


if(CHECK_CXX_COMPILER_USED_la_test)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (la_test
		m
	)
endif(CHECK_CXX_COMPILER_USED_la_test)

# Converter LIBSVM2BIN
file(GLOB_RECURSE LaSVM_LIBSVM2BIN_HEADERS
)

file(GLOB_RECURSE LaSVM_LIBSVM2BIN_SRC
	"${LaSVM_SOURCE_DIR}/src/utils/libsvm2bin.cpp"
)

add_executable(libsvm2bin ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_LIBSVM2BIN_HEADERS} ${LaSVM_LIBSVM2BIN_SRC})
target_link_libraries (libsvm2bin
	lasvm io
)
target_link_libraries (libsvm2bin
		${Boost_LIBRARIES}
	)


if(CHECK_CXX_COMPILER_USED_LIBSVM2BIN)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (libsvm2bin
		m
	)
endif(CHECK_CXX_COMPILER_USED_LIBSVM2BIN)

# Converter BIN2LIBSVM
file(GLOB_RECURSE LaSVM_BIN2LIBSVM_HEADERS
)

file(GLOB_RECURSE LaSVM_BIN2LIBSVM_SRC
	"${LaSVM_SOURCE_DIR}/src/utils/bin2libsvm.cpp"
)

add_executable(bin2libsvm ${Boost_INCLUDE_DIR} ${Boost_LIBRARY_DIR} ${LaSVM_TOOLS_HEADERS} ${LaSVM_IN_OUT_HEADERS} ${LaSVM_BIN2LIBSVM_HEADERS} ${LaSVM_BIN2LIBSVM_SRC})
target_link_libraries (bin2libsvm
	lasvm io
)
target_link_libraries (bin2libsvm
		${Boost_LIBRARIES}
	)


if(CHECK_CXX_COMPILER_USED_BIN2LIBSVM)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (bin2libsvm
		m
	)
endif(CHECK_CXX_COMPILER_USED_BIN2LIBSVM)
