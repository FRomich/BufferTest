cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Buff_test_cmd")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Buff_test_cmd 
    "Buff_test_cmd.cpp" 
    "Frame.h" 
    "FrameBuffer.h" "FrameBuffer.cpp"
    "Camera.h" "Camera.cpp"
    "CameraManager.h" "CameraManager.cpp" 
    "FrameProcessor.h" "FrameProcessor.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Buff_test_cmd PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
# MVS
set(MVS_ROOT "C:/Program Files (x86)/MVS/Development")
set(MVS_RUNTIME "C:/Program Files (x86)/Common Files/MVS/Runtime/Win64_x64")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${MVS_ROOT}/Includes"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${MVS_ROOT}/Libraries/win64/MvCameraControl.lib"
)

 add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MVS_RUNTIME}/MvCameraControl.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MVS_RUNTIME}/MVGigEVisionSDK.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
 )