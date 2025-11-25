cmake_minimum_required(VERSION 3.19.7)
project(nnv)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
##############
## Protobuf ##
##############
# Protobuf is needed to compile ONNX
set(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR}/dep/)
set(PROTOBUF_VERSION 3.19.2)
set(PROTOBUF_DEFAULT_DIR "${TOOLS_DIR}/protobuf-${PROTOBUF_VERSION}")
if (NOT PROTOBUF_DIR)
    set(PROTOBUF_DIR ${PROTOBUF_DEFAULT_DIR})
endif()

if(NOT EXISTS "${PROTOBUF_DIR}/installed/lib/libprotobuf.a")
    message("Can't find protobuf, installing. If protobuf is installed please use the PROTOBUF_DIR parameter to pass the path")
    if (${PROTOBUF_DIR} STREQUAL ${PROTOBUF_DEFAULT_DIR})
        message("installing protobuf")
        execute_process(COMMAND ${TOOLS_DIR}/download_protobuf.sh ${PROTOBUF_VERSION})
    else()
        message(FATAL_ERROR "Can't find protobuf in the supplied directory")
    endif()
endif()

set(PROTOBUF_LIB protobuf)
add_library(${PROTOBUF_LIB} SHARED IMPORTED)
set_property(TARGET ${PROTOBUF_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROTOBUF_LIB} PROPERTIES IMPORTED_LOCATION ${PROTOBUF_DIR}/installed/lib/libprotobuf.a)
target_include_directories(${PROTOBUF_LIB} INTERFACE ${PROTOBUF_DIR}/installed/include)
list(APPEND LIBS ${PROTOBUF_LIB})

# ###########
### ONNX ##
###########
set(ONNX_VERSION 1.15.0)
set(ONNX_DIR "${TOOLS_DIR}/onnx-${ONNX_VERSION}")

if(NOT EXISTS "${ONNX_DIR}/onnx.proto3.pb.h")
    message("generating ONNX protobuf file")
    execute_process(COMMAND ${TOOLS_DIR}/download_onnx.sh ${ONNX_VERSION} ${PROTOBUF_VERSION})
endif()
file(GLOB DEPS_ONNX "${ONNX_DIR}/*.cc")
include_directories(SYSTEM ${ONNX_DIR})

# Add executable
add_executable(nnv ./src/main.cpp ${DEPS_ONNX})

target_include_directories(nnv PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(nnv PRIVATE ${PROTOBUF_DIR}/installed/include)
target_link_libraries(nnv PRIVATE ${PROTOBUF_LIB})
