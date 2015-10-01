cmake_minimum_required (VERSION 3.2)
project (SNR)

include_directories (".")

add_executable (SNR SNR.cpp)

find_package(OpenEXR HINTS .)
if ( OPENEXR_FOUND )
   include_directories ( ${OPENEXR_INCLUDE_PATH} )
   target_link_libraries(SNR ${OPENEXR_LIBRARIES} )
   message ( STATUS " OPENEXR_INCLUDE_DIR: " ${OPENEXR_INCLUDE_PATH} )
   message ( STATUS " OPENEXR_LIBRARIES: " ${OPENEXR_LIBRARIES} )
endif ( OPENEXR_FOUND )

find_package(OpenMP)
if ( OPENMP_FOUND )
   target_compile_options(OpenMP_CXX_FLAGS)
   set_target_properties(SNR LINK_FLAGS OpenMP_CXX_FLAGS)
endif( OPENMP_FOUND )

find_package(Boost)
if ( Boost_FOUND )
   include_directories ( ${Boost_INCLUDE_DIRS} )
   link_directories (${Boost_LIBRARY_DIRS} )
   target_link_libraries (SNR ${Boost_LIBRARIES} )
   message ( status " BOOST_INCLUDE_DIR: " ${Boost_INCLUDE_DIRS} )
   message ( status " BOOST_LIBRARIES: " ${Boost_LIBRARIES} )
endif ( Boost_FOUND )

# Add C++11 features
target_compile_features(SNR PRIVATE cxx_range_for)

