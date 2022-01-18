cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(bayan VERSION ${PROJECT_VESRION})

add_executable(bayan main.cpp
  Hashing.h
  Hashing.cpp
  FileCollector.h
  FileInfo.h
  FileDuplicatesSearher.h
)

set_target_properties(bayan PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

if (MSVC)
    target_compile_options(bayan PRIVATE
        /W4 /WX
        -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING  # Supress warning in boost::bimap
    )    
else ()
    target_compile_options(bayan PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71.0 REQUIRED 
    COMPONENTS 
    program_options 
    filesystem 
    regex
)

if(NOT ${Boost_FOUND})
    message(FATAL_ERROR "Boost not found")
endif()

target_link_libraries(bayan 
    Boost::filesystem 
    Boost::regex
    Boost::program_options
)

install(TARGETS bayan RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
