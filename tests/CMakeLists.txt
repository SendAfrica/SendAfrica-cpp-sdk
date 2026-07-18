add_executable(sendafrica_tests test_local.cpp)
target_link_libraries(sendafrica_tests PRIVATE sendafrica::sendafrica)
add_test(NAME sendafrica_tests COMMAND sendafrica_tests)
