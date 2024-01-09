set(CUR_DIR ${CMAKE_CURRENT_LIST_DIR})
set(APP_SRC_FILES ${CUR_DIR}/main.cpp)
set(APP_OUTPUT_FILE breakout)
add_executable(${APP_OUTPUT_FILE} ${APP_SRC_FILES})

set_property(TARGET ${APP_OUTPUT_FILE} PROPERTY CXX_STANDARD 23)
target_include_directories(${APP_OUTPUT_FILE} PRIVATE ${THIRD_PARTY_DIR}/SDL/include/)
target_link_libraries(${APP_OUTPUT_FILE} PRIVATE debug ${THIRD_PARTY_DIR}/SDL/build/debug/SDL3.lib optimized ${THIRD_PARTY_DIR}/SDL/build/release/SDL3.lib)
add_custom_command(TARGET ${APP_OUTPUT_FILE} POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy 
     ${THIRD_PARTY_DIR}/SDL/build/$<CONFIGURATION>/SDL3.dll
     SDL3.dll
  COMMENT "Copying SDL3.dll to output directory..")
