cmake_minimum_required(VERSION 3.20)
project(GateServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS beast)
find_package(Boost REQUIRED COMPONENTS asio)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Boost REQUIRED COMPONENTS property_tree)
find_package(jsoncpp CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)

# protoc 编译 message.proto
find_program(_GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/message.proto")
set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h")
set(GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc")
set(GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h")

add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
    COMMAND protobuf::protoc
    ARGS --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
         --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf and gRPC C++ sources from message.proto"
)

add_executable(${PROJECT_NAME}
    GateServer.cpp
    CServer.cpp
    HttpConnection.cpp
    LogicSystem.cpp
    VerifyGrpcClient.cpp
    ConfigMgr.cpp
    ${PROTO_SRC}
    ${GRPC_SRC} 
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::beast
    Boost::asio
    Boost::filesystem
    Boost::property_tree
    JsonCpp::JsonCpp
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    hiredis::hiredis
)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.ini"
    COMMENT "Copying config.ini to output directory"
)