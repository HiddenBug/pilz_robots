cmake_minimum_required(VERSION 2.8.3)
project(prbt_hardware_support)

find_package(catkin REQUIRED COMPONENTS
  canopen_chain_node
  message_filters
  message_generation
  pilz_utils
  roscpp
  std_msgs
  std_srvs
  sensor_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  urdf
  dynamic_reconfigure
)

add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-Wno-unused-parameter)
add_definitions(-Werror)
add_definitions(-std=c++11)
add_definitions(-Wconversion) # At least this line needs to be below find_package to keep
                              #  the flag away from the gmock build
add_definitions(-Wpedantic)

# message generation
add_message_files(
  FILES
  BrakeTestErrorCodes.msg
  FrameSpeeds.msg
  ModbusMsgInStamped.msg
  ModbusRegisterBlock.msg
  OperationModes.msg
)

# action generation
add_service_files(
  FILES
  BrakeTest.srv
  GetOperationMode.srv
  GetSpeedOverride.srv
  IsBrakeTestRequired.srv
  SendBrakeTestResult.srv
  SetSpeedLimit.srv
  WriteModbusRegister.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

generate_dynamic_reconfigure_options(
  cfg/FakeSpeedOverride.cfg
)

###################################
## catkin specific configuration ##
###################################
catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS message_runtime std_msgs std_srvs sensor_msgs
)

################
## Clang tidy ##
################
if(CATKIN_ENABLE_CLANG_TIDY)
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
    )
  if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.")
  else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()


###########
## Build ##
###########

include_directories(
  include
  test/include
)
include_directories(SYSTEM ${catkin_INCLUDE_DIRS}) #Must be declared SYSTEM to avoid Warnings from ros system includes
include_directories(${pilz_utils_INCLUDE_DIRS})


# MODBUS_ADAPTER_STO_NODE
add_executable(modbus_adapter_sto_node
  src/modbus_adapter_sto_node.cpp
  src/modbus_adapter_sto.cpp
  src/modbus_msg_sto_wrapper.cpp
)
add_dependencies(modbus_adapter_sto_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(modbus_adapter_sto_node ${catkin_LIBRARIES})

# STOP1_EXECUTOR_NODE
add_executable(stop1_executor_node
  src/stop1_executor.cpp
  src/stop1_executor_node.cpp
)
add_dependencies(stop1_executor_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(stop1_executor_node ${catkin_LIBRARIES})

# OPERATION_MODE_SETUP_EXECUTOR_NODE
add_executable(operation_mode_setup_executor_node
  src/operation_mode_setup_executor.cpp
  src/operation_mode_setup_executor_node.cpp
)
add_dependencies(operation_mode_setup_executor_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(operation_mode_setup_executor_node ${catkin_LIBRARIES})

# BRAKE_TEST_NODE
add_executable(modbus_adapter_brake_test_node
  src/modbus_adapter_brake_test_node.cpp
  src/modbus_adapter_brake_test.cpp
)
add_dependencies(modbus_adapter_brake_test_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(modbus_adapter_brake_test_node ${catkin_LIBRARIES})

add_executable(modbus_adapter_operation_mode_node
  src/modbus_adapter_operation_mode_node.cpp
  src/modbus_adapter_operation_mode.cpp
  src/adapter_operation_mode.cpp
)

add_dependencies(modbus_adapter_operation_mode_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(modbus_adapter_operation_mode_node ${catkin_LIBRARIES})

add_executable(brake_test_executor_node
  src/brake_test_executor_node.cpp
  src/brake_test_executor.cpp
)
add_dependencies(brake_test_executor_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(brake_test_executor_node ${catkin_LIBRARIES})

add_executable(canopen_braketest_adapter_node
  src/canopen_braketest_adapter_node.cpp
  src/canopen_braketest_adapter.cpp
  src/canopen_braketest_adapter_exception.cpp
)
add_dependencies(canopen_braketest_adapter_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(canopen_braketest_adapter_node ${catkin_LIBRARIES})

# SYSTEM_INFO_NODE
add_executable(system_info_node
  src/system_info_node.cpp
  src/system_info.cpp
  src/system_info_exception.cpp
)
add_dependencies(system_info_node ${catkin_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(system_info_node ${catkin_LIBRARIES})

add_executable(fake_speed_override_node
  src/fake_speed_override_node.cpp
)
add_dependencies(fake_speed_override_node
  ${catkin_EXPORTED_TARGETS}
  ${PROJECT_NAME}_gencfg
  ${PROJECT_NAME}_generate_messages_cpp
)
target_link_libraries(fake_speed_override_node ${catkin_LIBRARIES})

# +++++++++++++++++++++++++++++++++
# + Build modbus client node +
# +++++++++++++++++++++++++++++++++
add_executable(
  pilz_modbus_client_node
  src/pilz_modbus_client_node.cpp
  src/pilz_modbus_client.cpp
  src/libmodbus_client.cpp
  src/modbus_msg_in_builder.cpp
)
add_dependencies(pilz_modbus_client_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(pilz_modbus_client_node ${catkin_LIBRARIES} modbus)


# +++++++++++++++++++++++++++++
# + Build speed observer node +
# +++++++++++++++++++++++++++++
add_executable(
  speed_observer_node
  src/speed_observer_node.cpp
  src/speed_observer.cpp
)
add_dependencies(speed_observer_node ${${PROJECT_NAME}_EXPORTED_TARGETS})
target_link_libraries(speed_observer_node ${catkin_LIBRARIES})

#############
## Install ##
#############

install(DIRECTORY launch DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
install(DIRECTORY config DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE)

install(TARGETS
  modbus_adapter_sto_node
  pilz_modbus_client_node
  stop1_executor_node
  canopen_braketest_adapter_node
  modbus_adapter_brake_test_node
  brake_test_executor_node
  modbus_adapter_operation_mode_node
  system_info_node
  speed_observer_node
  fake_speed_override_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

#############
## Testing ##
#############
if(CATKIN_ENABLE_TESTING)
  find_package(rostest REQUIRED)
  find_package(code_coverage REQUIRED)
  find_package(pilz_testutils REQUIRED)

  include_directories(${pilz_testutils_INCLUDE_DIRS})

  #catkin_lint: ignore_once missing_directory <-- needed for successful catkin_linting
  include_directories(SYSTEM ${gmock_SOURCE_DIR}/include/) #Declare system to supress strict conversion errors in gmock

  add_rostest_gmock(unittest_update_filter
                    test/unittests/unittest_update_filter.test
                    test/unittests/unittest_update_filter.cpp
  )
  target_link_libraries(unittest_update_filter
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_update_filter ${${PROJECT_NAME}_EXPORTED_TARGETS})

  catkin_add_gtest(unittest_libmodbus_client
      test/unittests/unittest_libmodbus_client.cpp
      test/unittests/pilz_modbus_server_mock.cpp
      src/libmodbus_client.cpp
  )
  target_link_libraries(unittest_libmodbus_client
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
    modbus
  )
  add_dependencies(unittest_libmodbus_client ${${PROJECT_NAME}_EXPORTED_TARGETS})

  catkin_add_gmock(unittest_modbus_api_spec
  test/unittests/unittest_modbus_api_spec.cpp
  )
  target_link_libraries(unittest_modbus_api_spec
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_modbus_api_spec ${${PROJECT_NAME}_EXPORTED_TARGETS})

  catkin_add_gtest(unittest_pilz_modbus_exception
                   test/unittests/unittest_pilz_modbus_exception.cpp)
  add_dependencies(unittest_pilz_modbus_exception ${${PROJECT_NAME}_EXPORTED_TARGETS})

  #--- ModbusMsgWrapper unit test ---
  catkin_add_gtest(unittest_modbus_msg_wrapper
                   test/unittests/unittest_modbus_msg_wrapper.cpp
                   src/modbus_msg_in_builder.cpp
                   )
  add_dependencies(unittest_modbus_msg_wrapper ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------

  catkin_add_gtest(unittest_pilz_modbus_client_exception
  test/unittests/unittest_pilz_modbus_client_exception.cpp)
  add_dependencies(unittest_pilz_modbus_client_exception ${${PROJECT_NAME}_EXPORTED_TARGETS})

  #--- PilzModbusClient unit test ---
  add_rostest_gmock(unittest_pilz_modbus_client
      test/unittests/unittest_pilz_modbus_client.test
      test/unittests/unittest_pilz_modbus_client.cpp
      src/pilz_modbus_client.cpp
      src/modbus_msg_in_builder.cpp
  )
  target_link_libraries(unittest_pilz_modbus_client
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_pilz_modbus_client ${${PROJECT_NAME}_EXPORTED_TARGETS})

  # --- ModbusAdapterBrakeTest unit test ---
  catkin_add_gmock(unittest_modbus_adapter_brake_test
    test/unittests/unittest_modbus_adapter_brake_test.cpp
    src/modbus_adapter_brake_test.cpp
    src/modbus_msg_in_builder.cpp
  )
  target_link_libraries(unittest_modbus_adapter_brake_test
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- ModbusAdapterOperationMode unit test ---
  add_rostest_gmock(unittest_modbus_adapter_operation_mode
    test/unittests/unittest_modbus_adapter_operation_mode.test
    test/unittests/unittest_modbus_adapter_operation_mode.cpp
    src/modbus_adapter_operation_mode.cpp
    src/adapter_operation_mode.cpp
    src/modbus_msg_in_builder.cpp
  )
  target_link_libraries(unittest_modbus_adapter_operation_mode
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- BrakeTestUtils unit test ---
  add_rostest_gtest(unittest_brake_test_utils
    test/unittests/unittest_brake_test_utils.test
    test/unittests/unittest_brake_test_utils.cpp
    test/unittests/joint_states_publisher_mock.cpp
  )
  target_link_libraries(unittest_brake_test_utils
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- CanOpenBraketestAdapter unit test ---
  add_rostest_gmock(unittest_canopen_braketest_adapter
    test/unittests/unittest_canopen_braketest_adapter.test
    test/unittests/unittest_canopen_braketest_adapter.cpp
    test/unittests/canopen_chain_node_mock.cpp
    src/canopen_braketest_adapter.cpp
    src/canopen_braketest_adapter_exception.cpp
  )
  target_link_libraries(unittest_canopen_braketest_adapter
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- BraketestExecutor unit test ---
  catkin_add_gmock(unittest_brake_test_executor
    test/unittests/unittest_brake_test_executor.cpp
    src/brake_test_executor.cpp
  )
  target_link_libraries(unittest_brake_test_executor
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- Stop1Executor unit test ---
  catkin_add_gmock(unittest_stop1_executor
    src/stop1_executor.cpp
    test/unittests/unittest_stop1_executor.cpp
  )
  target_link_libraries(unittest_stop1_executor
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  #--- StoModbusAdapter intrgration test ---
  catkin_add_gmock(unittest_modbus_adapter_sto
    test/unittests/unittest_modbus_adapter_sto.cpp
    src/modbus_adapter_sto.cpp
    src/modbus_msg_sto_wrapper.cpp
    src/modbus_msg_in_builder.cpp
  )
  target_link_libraries(unittest_modbus_adapter_sto
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  # --- Stop integration test ---
  add_rostest_gmock(integrationtest_stop1
     test/integrationtests/integrationtest_stop1.test
     test/integrationtests/integrationtest_stop1.cpp
     test/unittests/pilz_modbus_server_mock.cpp
     src/libmodbus_client.cpp
  )
  target_link_libraries(integrationtest_stop1
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
    modbus
  )
  add_dependencies(integrationtest_stop1 ${catkin_EXPORTED_TARGETS})
  #----------------------------------

  # --- Stop1 missing service integration tests ---
  add_rostest_gtest(integrationtest_stop1_service_missing
     test/integrationtests/integrationtest_stop1_service_missing.test
     test/integrationtests/integrationtest_stop1_service_missing.cpp
  )
  target_link_libraries(integrationtest_stop1_service_missing
    ${catkin_LIBRARIES}
  )
  add_dependencies(integrationtest_stop1_service_missing ${catkin_EXPORTED_TARGETS})

  foreach(service_name halt hold unhold)
    add_rostest(test/integrationtests/integrationtest_stop1_service_missing.test
      ARGS omit_service:=${service_name}
      DEPENDENCIES integrationtest_stop1_service_missing)
  endforeach(service_name)
  #----------------------------------

  # --- Brake test required integration test ---
  add_rostest_gmock(integrationtest_brake_test_required
    test/integrationtests/integrationtest_brake_test_required.test
    test/integrationtests/integrationtest_brake_test_required.cpp
    test/unittests/pilz_modbus_server_mock.cpp
    src/libmodbus_client.cpp
  )
  target_link_libraries(integrationtest_brake_test_required
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
    modbus
  )
  add_dependencies(integrationtest_brake_test_required ${catkin_EXPORTED_TARGETS})
  #----------------------------------

  # --- Operation mode integration test ---
  add_rostest_gmock(integrationtest_operation_mode
    test/integrationtests/integrationtest_operation_mode.test
    test/integrationtests/integrationtest_operation_mode.cpp
    test/unittests/pilz_modbus_server_mock.cpp
    src/libmodbus_client.cpp
  )
  target_link_libraries(integrationtest_operation_mode
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
    modbus
  )
  add_dependencies(integrationtest_operation_mode ${catkin_EXPORTED_TARGETS})
  #----------------------------------

  # --- Execute brake test integration test ---
  add_rostest_gmock(integrationtest_execute_brake_test
    test/integrationtests/integrationtest_execute_brake_test.test
    test/integrationtests/integrationtest_execute_brake_test.cpp
    test/unittests/joint_states_publisher_mock.cpp
    test/unittests/pilz_modbus_server_mock.cpp
    test/unittests/canopen_chain_node_mock.cpp
    src/libmodbus_client.cpp
  )
  target_link_libraries(integrationtest_execute_brake_test
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
    modbus
  )
  add_dependencies(integrationtest_execute_brake_test ${catkin_EXPORTED_TARGETS})
  #----------------------------------------

  # --- Speed Observer unit test ---
  add_rostest_gmock(unittest_speed_observer
      test/unittests/unittest_speed_observer.test
      test/unittests/unittest_speed_observer.cpp
      src/speed_observer.cpp
  )
  target_link_libraries(unittest_speed_observer
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_speed_observer ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  # --- Speed Observer and Operation Mode integration test ---
  add_rostest_gmock(integrationtest_speed_observer_and_operation_mode
      test/integrationtests/integrationtest_speed_observer_and_operation_mode.test
      test/integrationtests/integrationtest_speed_observer_and_operation_mode.cpp
  )
  target_link_libraries(integrationtest_speed_observer_and_operation_mode
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(integrationtest_speed_observer_and_operation_mode ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  #--- SystemInfo unit test ---
  add_rostest_gmock(unittest_system_info
      test/unittests/unittest_system_info.test
      test/unittests/unittest_system_info.cpp
      src/system_info.cpp
      src/system_info_exception.cpp
  )
  target_link_libraries(unittest_system_info
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_system_info ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  # --- FilterPipeline unit test ---
  add_rostest_gtest(unittest_filter_pipeline
    test/unittests/unittest_filter_pipeline.test
    test/unittests/unittest_filter_pipeline.cpp
  )
  target_link_libraries(unittest_filter_pipeline
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  #----------------------------------

  #--- ModbusMsgInUtils unit test ---
  catkin_add_gtest(unittest_modbus_msg_in_builder
      test/unittests/unittest_modbus_msg_in_builder.cpp
      src/modbus_msg_in_builder.cpp
  )
  target_link_libraries(unittest_modbus_msg_in_builder
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_modbus_msg_in_builder ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  #--- OperationModeSetupExecutor unit test ---
  catkin_add_gmock(unittest_operation_mode_setup_executor
      test/unittests/unittest_operation_mode_setup_executor.cpp
      src/operation_mode_setup_executor.cpp
  )
  target_link_libraries(unittest_operation_mode_setup_executor
    ${catkin_LIBRARIES} ${pilz_testutils_LIBRARIES}
  )
  add_dependencies(unittest_operation_mode_setup_executor ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  #--- OperationModeSetupExecutor unit test ---
  add_rostest_gtest(unittest_fake_speed_override
      test/unittests/unittest_fake_speed_override.test
      test/unittests/unittest_fake_speed_override.cpp
  )
  target_link_libraries(unittest_fake_speed_override
    ${catkin_LIBRARIES}
  )
  add_dependencies(unittest_fake_speed_override ${${PROJECT_NAME}_EXPORTED_TARGETS})
  #----------------------------------------

  #--- utils unittest ---
  catkin_add_gtest(unittest_utils
    test/unittests/unittest_utils.cpp
  )
  target_link_libraries(unittest_utils
    ${catkin_LIBRARIES}
  )
  #----------------------------------------

  # to run: catkin_make -DENABLE_COVERAGE_TESTING=ON package_name_coverage (adding -j1 recommended)
  if(ENABLE_COVERAGE_TESTING)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES "*/${PROJECT_NAME}/test*"
                          "*/BrakeTestErrorCodes.h"
                          "*/ModbusMsgInStamped.h"
                          "*/BrakeTest.h"
                          "*/BrakeTestRequest.h"
                          "*/BrakeTestResponse.h"
                          "*/IsBrakeTestRequired.h"
                          "*/IsBrakeTestRequiredRequest.h"
                          "*/IsBrakeTestRequiredResponse.h"
                          "*/ModbusRegisterBlock.h"
                          "*/WriteModbusRegister.h"
                          "*/WriteModbusRegisterRequest.h"
                          "*/WriteModbusRegisterResponse.h"
                          "*/GetOperationMode.h"
                          "*/GetOperationModeRequest.h"
                          "*/GetOperationModeResponse.h"
                          "*/GetSpeedOverride.h"
                          "*/GetSpeedOverrideRequest.h"
                          "*/GetSpeedOverrideResponse.h"
                          "*/OperationModes.h"
                          "*/SendBrakeTestResult.h"
                          "*/SendBrakeTestResultRequest.h"
                          "*/SendBrakeTestResultResponse.h"
                          "*/FrameSpeeds.h"
                          "*/SetSpeedLimit.h"
                          "*/SetSpeedLimitRequest.h"
                          "*/SetSpeedLimitResponse.h"
                          "*/FakeSpeedOverrideConfig.h"
                          )
    add_code_coverage(
      NAME ${PROJECT_NAME}_coverage
      DEPENDS tests
    )
  endif()
endif()
