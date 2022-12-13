add_executable(dbSSD-host "dbSSD-host.cpp" "table.cpp" "NVMEDevice.cpp")

add_custom_command(
    TARGET dbSSD-host POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/testTables/*
            ${CMAKE_CURRENT_BINARY_DIR}/
)

set_property(TARGET dbSSD-host PROPERTY CXX_STANDARD 20)
target_link_libraries(dbSSD-host "nvme")
