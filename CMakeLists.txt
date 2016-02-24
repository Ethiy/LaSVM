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


#TOOLS
file(GLOB_RECURSE LaSVM_TOOLS_HEADERS
	"${LaSVM_SOURCE_DIR}/*.hpp"
)

file(GLOB_RECURSE LaSVM_TOOLS_SRC
	"${LaSVM_SOURCE_DIR}/*.cc"
)

add_library(Tools ${LaSVM_TOOLS_HEADERS} ${LaSVM_TOOLS_SRC})

if(CHECK_CXX_COMPILER_USED_TOOLS)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (Tools
		m
	)
endif(CHECK_CXX_COMPILER_USED_TOOLS)

#Binaries
# Training
file(GLOB_RECURSE LaSVM_Training_HEADERS
)

file(GLOB_RECURSE LaSVM_Training_SRC
	"la_svm.cpp"
)

add_executable(la_svm ${LaSVM_TOOLS_HEADERS} ${LaSVM_Training_HEADERS} ${LaSVM_Training_SRC})
target_link_libraries (la_svm
	Tools
)

if(CHECK_CXX_COMPILER_USED_Training)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (la_svm
		m
	)
endif(CHECK_CXX_COMPILER_USED_Training)

# Test
file(GLOB_RECURSE LaSVM_Test_HEADERS
)

file(GLOB_RECURSE LaSVM_Test_SRC
	"la_test.cpp"
)

add_executable(la_test ${LaSVM_TOOLS_HEADERS} ${LaSVM_Test_HEADERS} ${LaSVM_Test_SRC})
target_link_libraries (la_test
	Tools
)

if(CHECK_CXX_COMPILER_USED_Test)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (la_test
		m
	)
endif(CHECK_CXX_COMPILER_USED_Test)

# Converter LIBSVM2BIN
file(GLOB_RECURSE LaSVM_LIBSVM2BIN_HEADERS
)

file(GLOB_RECURSE LaSVM_LIBSVM2BIN_SRC
	"libsvm2bin.cpp"
)

add_executable(LIBSVM2BIN ${LaSVM_TOOLS_HEADERS} ${LaSVM_LIBSVM2BIN_HEADERS} ${LaSVM_LIBSVM2BIN_SRC})
target_link_libraries (LIBSVM2BIN
	Tools
)

if(CHECK_CXX_COMPILER_USED_LIBSVM2BIN)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (LIBSVM2BIN
		m
	)
endif(CHECK_CXX_COMPILER_USED_LIBSVM2BIN)

# Converter BIN2LIBSVM
file(GLOB_RECURSE LaSVM_BIN2LIBSVM_HEADERS
)

file(GLOB_RECURSE LaSVM_BIN2LIBSVM_SRC
	"bin2libsvm.cpp"
)

add_executable(BIN2LIBSVM ${LaSVM_TOOLS_HEADERS} ${LaSVM_BIN2LIBSVM_HEADERS} ${LaSVM_BIN2LIBSVM_SRC})
target_link_libraries (BIN2LIBSVM
	Tools
)

if(CHECK_CXX_COMPILER_USED_BIN2LIBSVM)

elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "MSVCx")
  # using Visual Studio C++
elseif("${CMAKE_CXX_COMPILER_ID}x" STREQUAL "Intelx")
  # using Intel C++
else()
  # GCC or Clang
	target_link_libraries (BIN2LIBSVM
		m
	)
endif(CHECK_CXX_COMPILER_USED_BIN2LIBSVM)
