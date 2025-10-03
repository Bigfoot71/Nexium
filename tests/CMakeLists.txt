set(RESOURCES_PATH "${HP_ROOT_PATH}/tests/resources/")
set(INCLUDE_PATH "${HP_ROOT_PATH}/include")

function(add_hyperion_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE hyperion)
    target_compile_definitions(${test_name} PRIVATE RESOURCES_PATH="${RESOURCES_PATH}")
    target_include_directories(${test_name} PRIVATE "include")
    if(HP_HAVE_LIB_M)
        target_link_libraries(${test_name} PUBLIC m)
    endif()
endfunction()

add_hyperion_test("hp-skybox-procedural" "${HP_ROOT_PATH}/tests/skybox_procedural.c")
add_hyperion_test("hp-material-shader" "${HP_ROOT_PATH}/tests/material_shader.c")
add_hyperion_test("hp-render-texture" "${HP_ROOT_PATH}/tests/render_texture.c")
add_hyperion_test("hp-animation" "${HP_ROOT_PATH}/tests/animation.c")
add_hyperion_test("hp-instanced" "${HP_ROOT_PATH}/tests/instanced.c")
add_hyperion_test("hp-billboard" "${HP_ROOT_PATH}/tests/billboard.c")
add_hyperion_test("hp-streams" "${HP_ROOT_PATH}/tests/streams.c")
add_hyperion_test("hp-overlay" "${HP_ROOT_PATH}/tests/overlay.c")
add_hyperion_test("hp-skybox" "${HP_ROOT_PATH}/tests/skybox.c")
add_hyperion_test("hp-bloom" "${HP_ROOT_PATH}/tests/bloom.c")
add_hyperion_test("hp-ssao" "${HP_ROOT_PATH}/tests/ssao.c")
add_hyperion_test("hp-fog" "${HP_ROOT_PATH}/tests/fog.c")
add_hyperion_test("hp-pbr" "${HP_ROOT_PATH}/tests/pbr.c")

if(WIN32)
    set(vendored_dirs
        "${CMAKE_BINARY_DIR}/external/SDL"
        "${CMAKE_BINARY_DIR}/external/physfs"
        "${CMAKE_BINARY_DIR}/external/freetype"
        "${CMAKE_BINARY_DIR}/external/assimp"
    )
    foreach(dir IN LISTS vendored_dirs)
        if(EXISTS "${dir}")
            file(GLOB_RECURSE dll_files "${dir}/*.dll")
            foreach(dll_file ${dll_files})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${dll_file}"
                            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                    COMMENT "Copying ${dll_file}"
                )
            endforeach()
        endif()
    endforeach()
endif()
