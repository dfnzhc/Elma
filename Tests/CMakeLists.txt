add_executable(test_filter filter.cpp)
target_link_libraries(test_filter ElmaLib)
add_test(filter test_filter)
set_tests_properties(filter PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_frame frame.cpp)
target_link_libraries(test_frame ElmaLib)
add_test(frame test_frame)
set_tests_properties(frame PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_image image.cpp)
target_link_libraries(test_image ElmaLib)
add_test(image test_image)
set_tests_properties(image PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_intersection intersection.cpp)
target_link_libraries(test_intersection ElmaLib)
add_test(intersection test_intersection)
set_tests_properties(intersection PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_materials materials.cpp)
target_link_libraries(test_materials ElmaLib)
add_test(materials test_materials)
set_tests_properties(materials PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_matrix matrix.cpp)
target_link_libraries(test_matrix ElmaLib)
add_test(matrix test_matrix)
set_tests_properties(matrix PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")

add_executable(test_mipmap mipmap.cpp)
target_link_libraries(test_mipmap ElmaLib)
add_test(mipmap test_mipmap)
set_tests_properties(mipmap PROPERTIES PASS_REGULAR_EXPRESSION "SUCCESS")
