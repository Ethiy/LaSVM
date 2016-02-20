project(LaSVM CXX C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#TOOLS
file(GLOB_RECURSE LaSVM_TOOLS_HEADERS
	"${LaSVM_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE LaSVM_TOOLS_SRC
	"${LaSVM_SOURCE_DIR}/*.c"
)

add_library(Tools ${LaSVM_TOOLS_HEADERS} ${LaSVM_TOOLS_SRC})
target_link_libraries (Tools
	m
)

#Binaries
# Training
file(GLOB_RECURSE LaSVM_Training_HEADERS
)

file(GLOB_RECURSE LaSVM_Training_SRC
	"la_svm.cpp"
)

add_executable(Training ${LaSVM_TOOLS_HEADERS} ${LaSVM_Training_HEADERS} ${LaSVM_Training_SRC})
target_link_libraries (Training
	Tools
	m
)

# Test
file(GLOB_RECURSE LaSVM_Test_HEADERS
)

file(GLOB_RECURSE LaSVM_Test_SRC
	"la_test.cpp"
)

add_executable(Test ${LaSVM_TOOLS_HEADERS} ${LaSVM_Test_HEADERS} ${LaSVM_Test_SRC})
target_link_libraries (Test
	Tools
	m
)

# Converter LIBSVM2BIN
file(GLOB_RECURSE LaSVM_LIBSVM2BIN_HEADERS
)

file(GLOB_RECURSE LaSVM_LIBSVM2BIN_SRC
	"libsvm2bin.cpp"
)

add_executable(LIBSVM2BIN ${LaSVM_TOOLS_HEADERS} ${LaSVM_LIBSVM2BIN_HEADERS} ${LaSVM_LIBSVM2BIN_SRC})
target_link_libraries (LIBSVM2BIN
	Tools
	m
)

# Converter BIN2LIBSVM
file(GLOB_RECURSE LaSVM_BIN2LIBSVM_HEADERS
)

file(GLOB_RECURSE LaSVM_BIN2LIBSVM_SRC
	"bin2libsvm.cpp"
)

add_executable(BIN2LIBSVM ${LaSVM_TOOLS_HEADERS} ${LaSVM_BIN2LIBSVM_HEADERS} ${LaSVM_BIN2LIBSVM_SRC})
target_link_libraries (BIN2LIBSVM
	Tools
	m
)
