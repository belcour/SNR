cmake_minimum_required (VERSION 3.2)
project (SNR)

include_directories (".")

add_executable (SNR SNR.cpp)

find_package(OpenMP)
if ( OPENMP_FOUND )
	target_compile_options(SNR PUBLIC ${OpenMP_CXX_FLAGS})
	set_target_properties(SNR PROPERTIES LINK_FLAGS ${OpenMP_CXX_FLAGS})
endif( OPENMP_FOUND )

find_package(Boost COMPONENTS log REQUIRED)
if ( Boost_FOUND )
   target_compile_definitions(SNR PUBLIC BOOST_LOG_DYN_LINK)
   include_directories ( ${Boost_INCLUDE_DIRS} )
   link_directories (${Boost_LIBRARY_DIRS} )
   target_link_libraries (SNR ${Boost_LIBRARIES})
   message ( status " BOOST_INCLUDE_DIR: " ${Boost_INCLUDE_DIRS} )
   message ( status " BOOST_LIBRARIES: " ${Boost_LIBRARIES} )
endif ( Boost_FOUND )

find_package (Threads)
target_link_libraries (SNR ${CMAKE_THREAD_LIBS_INIT})

# Add C++11 features
target_compile_features(SNR PRIVATE cxx_range_for)
