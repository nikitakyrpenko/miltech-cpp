add_library(
    drone_service STATIC
    src/TargetProvider.cpp
    src/ConfigLoader.cpp
    src/FirepointProvider.cpp
    src/BallisticSolutionEvaluator.cpp
    src/MissionProccesor.cpp
    src/DroneProvider.cpp
    src/MissionFactory.cpp
    src/AnalyticalBallisticSolver.cpp
    src/TableBallisticSolver.cpp)

target_include_directories(
    drone_service
    PUBLIC include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external)

target_link_libraries(drone_service PUBLIC drone_model drone_state drone_utils)
target_compile_options(drone_service PRIVATE -Wall -Wextra -pedantic)

if(BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    # test sources are gitignored/untracked; only build the test target if they exist locally
    file(GLOB drone_service_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
    if(drone_service_test_sources)
        add_executable(drone_service_tests ${drone_service_test_sources})
        target_include_directories(drone_service_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        target_link_libraries(drone_service_tests PRIVATE GTest::gtest_main drone_service)
        target_compile_options(drone_service_tests PRIVATE -Wall -Wextra -pedantic)
        gtest_discover_tests(drone_service_tests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS
                                                                                    "drone_service")
    endif()
endif()
