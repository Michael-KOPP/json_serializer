cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

add_library(serialization INTERFACE serialization/serializer.hpp serialization/json_serializer.hpp serialization/traits.hpp)

find_package(refl CONFIG REQUIRED)

target_link_libraries(serialization INTERFACE refl::refl)

target_include_directories(serialization INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>   # Utilisé lors de la phase de build
    $<INSTALL_INTERFACE:include>  # Utilisé lors de l'installation
    )

install(DIRECTORY serialization DESTINATION include)

install(TARGETS serialization
        EXPORT serializationTargets
        INCLUDES DESTINATION include/serialization)

install(EXPORT serializationTargets
        FILE serializationConfig.cmake
        NAMESPACE serialization::
        DESTINATION lib/cmake/serialization)
