cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "" FORCE)

project(course_work LANGUAGES CXX)

find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)

qt_standard_project_setup()

qt_add_executable(course_work
    main.cpp
    core/monitor.h core/monitor.cpp
    core/dnsbench.h core/dnsbench.cpp
    core/errorhandler.h core/errorhandler.cpp
    core/failover.h core/failover.cpp
    core/qosscheduler.h core/qosscheduler.cpp
    core/selfhealing.h core/selfhealing.cpp
    core/trafficinspect.h core/trafficinspect.cpp
    storage/database.h storage/database.cpp
    storage/eventlogger.h storage/eventlogger.cpp
    ipc/ipcserver.h ipc/ipcserver.cpp
    gui/ipc_client_test.py
    gui/main.py
    gui/tray.py
    gui/main_window.py
    gui/ping_chart.py
    gui/conn_viz.py
    gui/report_builder.py
)

target_link_libraries(course_work
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        spdlog::spdlog
)

target_include_directories(course_work PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/ipc
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

install(TARGETS course_work
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)