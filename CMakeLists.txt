cmake_minimum_required(VERSION 3.20)
project(nexium VERSION 0.1.0 LANGUAGES C CXX)

# Used language standards

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Hot reload for MSVC compilers if supported

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Determine the project's root directory and check if it's the main project

set(NX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(NX_IS_MAIN ON)
else()
    set(NX_IS_MAIN OFF)
endif()

# Nexium build config options

option(NX_BUILD_DOCS "Build documentation with Doxygen" OFF)
option(NX_BUILD_TESTS "Enable building tests" ${NX_IS_MAIN})
option(NX_BUILD_SHARED "Build Nexium as a shared library" OFF)
option(NX_INSTALL "Enable installation of the Nexium library" ${NX_IS_MAIN})

option(NX_SDL3_VENDORED "Build SDL3 from vendored submodule" OFF)
option(NX_ZLIB_VENDORED "Build zlib from vendored submodule" OFF)
option(NX_PHYSFS_VENDORED "Build PhysFS from vendored submodule" OFF)
option(NX_FREETYPE_VENDORED "Build Freetype from vendored submodule" OFF)
option(NX_ASSIMP_VENDORED "Build Assimp from vendored submodule" OFF)

# Shared build setup

if(NX_BUILD_SHARED)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_definitions(-DNX_BUILD_SHARED)
endif()

# Set the cmake module path

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include standard cmake scripts

include(CheckLibraryExists)
include(EmbedShaders)
include(EmbedAssets)

# External dependencies

set(NX_EXTERNAL_INSTALL_TARGETS "")
set(NX_EXTERNAL_LIBS "")

# Configure SDL

if(NX_SDL3_VENDORED)

    set(SDL_SHARED          ON CACHE BOOL "")

    set(SDL_STATIC          OFF CACHE BOOL "")
    set(SDL_INSTALL         OFF CACHE BOOL "")
    set(SDL_EXAMPLES        OFF CACHE BOOL "")
    set(SDL_TEST_LIBRARY    OFF CACHE BOOL "")

    set(SDL_VIDEO           ON CACHE BOOL "")
    set(SDL_AUDIO           ON CACHE BOOL "")
    set(SDL_JOYSTICK        ON CACHE BOOL "")
    set(SDL_OPENGL          ON CACHE BOOL "")

    set(SDL_DIALOG          OFF CACHE BOOL "")
    set(SDL_RENDER          OFF CACHE BOOL "")
    set(SDL_CAMERA          OFF CACHE BOOL "")
    set(SDL_HAPTIC          OFF CACHE BOOL "")
    set(SDL_POWER           OFF CACHE BOOL "")
    set(SDL_SENSOR          OFF CACHE BOOL "")
    set(SDL_GPU             OFF CACHE BOOL "")
    set(SDL_VULKAN          OFF CACHE BOOL "")
    set(SDL_DIRECTX         OFF CACHE BOOL "")
    set(SDL_OPENGLES        OFF CACHE BOOL "")
    set(SDL_OFFSCREEN       OFF CACHE BOOL "")

    add_subdirectory("${NX_ROOT_PATH}/external/SDL")

    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "SDL3_Headers")
    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "SDL3-shared")
    list(APPEND NX_EXTERNAL_LIBS "SDL3::SDL3")

else()
    find_package(SDL3 REQUIRED)
    list(APPEND NX_EXTERNAL_LIBS "SDL3::SDL3")
endif()

# Configure zlib

if(NX_ZLIB_VENDORED)

    set(ZLIB_BUILD_STATIC       ON CACHE BOOL "")

    set(ZLIB_BUILD_TESTING      OFF CACHE BOOL "")
    set(ZLIB_BUILD_SHARED       OFF CACHE BOOL "")
    set(ZLIB_BUILD_MINIZIP      OFF CACHE BOOL "")
    set(ZLIB_INSTALL            OFF CACHE BOOL "")
    set(ZLIB_PREFIX             OFF CACHE BOOL "")

    set(_OLD_PIC ${CMAKE_POSITION_INDEPENDENT_CODE})
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    add_subdirectory("${NX_ROOT_PATH}/external/zlib")
    set(CMAKE_POSITION_INDEPENDENT_CODE ${_OLD_PIC})

    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "zlibstatic")
    list(APPEND NX_EXTERNAL_LIBS "zlibstatic")

else()
    find_package(ZLIB REQUIRED)
    list(APPEND NX_EXTERNAL_LIBS ZLIB::ZLIB)
endif()

# Configure PhysFS

if(NX_PHYSFS_VENDORED)

    set(PHYSFS_BUILD_SHARED             ON CACHE BOOL "")

    set(PHYSFS_BUILD_STATIC             OFF CACHE BOOL "")
    set(PHYSFS_BUILD_TEST               OFF CACHE BOOL "")
    set(PHYSFS_DISABLE_INSTALL          OFF CACHE BOOL "")
    set(PHYSFS_BUILD_DOCS               OFF CACHE BOOL "")

    set(PHYSFS_ARCHIVE_ZIP              ON CACHE BOOL "")
    set(PHYSFS_ARCHIVE_7Z               ON CACHE BOOL "")
    set(PHYSFS_ARCHIVE_GRP              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_WAD              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_CSM              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_HOG              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_MVL              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_QPAK             OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_SLB              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_ISO9660          OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_VDF              OFF CACHE BOOL "")
    set(PHYSFS_ARCHIVE_LECARCHIVES      OFF CACHE BOOL "")

    add_subdirectory("${NX_ROOT_PATH}/external/physfs")

    set_target_properties(physfs PROPERTIES
        COMPILE_OPTIONS "-D_POSIX_C_SOURCE=200809L"
    )

    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "physfs")
    list(APPEND NX_EXTERNAL_LIBS "physfs")

else()
    find_package(PhysFS REQUIRED)
    list(APPEND NX_EXTERNAL_LIBS "PhysFS::PhysFS")
endif()

# Configure FreeType

if(NX_FREETYPE_VENDORED)

    set(FT_DISABLE_ZLIB                 ON CACHE BOOL "")
    set(FT_DISABLE_BZIP2                ON CACHE BOOL "")
    set(FT_DISABLE_PNG                  ON CACHE BOOL "")
    set(FT_DISABLE_HARFBUZZ             ON CACHE BOOL "")
    set(FT_DISABLE_BROTLI               ON CACHE BOOL "")

    add_subdirectory("${NX_ROOT_PATH}/external/freetype")

    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "freetype")
    list(APPEND NX_EXTERNAL_LIBS "freetype")

else()
    find_package(Freetype REQUIRED)
    list(APPEND NX_EXTERNAL_LIBS "Freetype::Freetype")
endif()

# Configure Assimp

if(NX_ASSIMP_VENDORED)

    set(ASSIMP_INJECT_DEBUG_POSTFIX     OFF CACHE BOOL "")
    set(ASSIMP_WARNINGS_AS_ERRORS       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_TESTS              OFF CACHE BOOL "")
    set(ASSIMP_INSTALL                  OFF CACHE BOOL "")

    set(ASSIMP_IGNORE_GIT_HASH          ON CACHE BOOL "")
    set(ASSIMP_NO_EXPORT                ON CACHE BOOL "")

    set(ASSIMP_BUILD_COLLADA_IMPORTER   ON CACHE BOOL "")
    set(ASSIMP_BUILD_GLTF_IMPORTER      ON CACHE BOOL "")
    set(ASSIMP_BUILD_OBJ_IMPORTER       ON CACHE BOOL "")
    set(ASSIMP_BUILD_FBX_IMPORTER       ON CACHE BOOL "")

    set(ASSIMP_BUILD_IQM_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_M3D_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_AMF_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_BVH_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_OFF_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_COB_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_STL_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_3DS_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_AC_IMPORTER        OFF CACHE BOOL "")
    set(ASSIMP_BUILD_ASE_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_ASSBIN_IMPORTER    OFF CACHE BOOL "")
    set(ASSIMP_BUILD_B3D_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_DXF_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_CSM_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_HMP_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_IRRMESH_IMPORTER   OFF CACHE BOOL "")
    set(ASSIMP_BUILD_IRR_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_LWO_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_LWS_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MD2_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MD3_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MD5_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MDC_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MDL_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_NFF_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_NDO_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_OGRE_IMPORTER      OFF CACHE BOOL "")
    set(ASSIMP_BUILD_OPENGEX_IMPORTER   OFF CACHE BOOL "")
    set(ASSIMP_BUILD_PLY_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MS3D_IMPORTER      OFF CACHE BOOL "")
    set(ASSIMP_BUILD_BLEND_IMPORTER     OFF CACHE BOOL "")
    set(ASSIMP_BUILD_IFC_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_XGL_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_Q3D_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_Q3BSP_IMPORTER     OFF CACHE BOOL "")
    set(ASSIMP_BUILD_RAW_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_SIB_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_SMD_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_TERRAGEN_IMPORTER  OFF CACHE BOOL "")
    set(ASSIMP_BUILD_3D_IMPORTER        OFF CACHE BOOL "")
    set(ASSIMP_BUILD_X_IMPORTER         OFF CACHE BOOL "")
    set(ASSIMP_BUILD_X3D_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_3MF_IMPORTER       OFF CACHE BOOL "")
    set(ASSIMP_BUILD_MMD_IMPORTER       OFF CACHE BOOL "")

    if(NX_ZLIB_VENDORED)
        # If zlib is already provided, we tell assimp not to build it
        set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
        # And we provide the information that assimp needs to use our zlib
        # Assimp does a find_package(ZLIB) so we "simulate" a result
        set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
        set(ZLIB_INCLUDE_DIR "${NX_ROOT_PATH}/external/zlib" CACHE PATH "" FORCE)
        set(ZLIB_LIBRARIES zlibstatic CACHE STRING "" FORCE)
    endif()

    add_subdirectory("${NX_ROOT_PATH}/external/assimp")

    list(APPEND NX_EXTERNAL_INSTALL_TARGETS "assimp")
    list(APPEND NX_EXTERNAL_LIBS "assimp")

elseif()
    find_package(assimp REQUIRED)
    list(APPEND NX_EXTERNAL_LIBS "assimp::assimp")
endif()

# List all the Nexium sources

set(NX_SOURCES

    "${NX_ROOT_PATH}/source/Detail/GPU/VertexArray.cpp"
    "${NX_ROOT_PATH}/source/Detail/GPU/Framebuffer.cpp"
    "${NX_ROOT_PATH}/source/Detail/GPU/Texture.cpp"
    "${NX_ROOT_PATH}/source/Detail/GPU/Buffer.cpp"

    "${NX_ROOT_PATH}/source/INX_GPUProgramCache.cpp"
    "${NX_ROOT_PATH}/source/INX_GlobalAssets.cpp"
    "${NX_ROOT_PATH}/source/INX_GlobalState.cpp"
    "${NX_ROOT_PATH}/source/INX_GlobalPool.cpp"
    "${NX_ROOT_PATH}/source/INX_Utils.cpp"

    "${NX_ROOT_PATH}/source/NX_ReflectionProbe.cpp"
    "${NX_ROOT_PATH}/source/NX_InstanceBuffer.cpp"
    "${NX_ROOT_PATH}/source/NX_RenderTexture.cpp"
    "${NX_ROOT_PATH}/source/NX_DynamicMesh.cpp"
    "${NX_ROOT_PATH}/source/NX_Environment.cpp"
    "${NX_ROOT_PATH}/source/NX_AudioStream.cpp"
    "${NX_ROOT_PATH}/source/NX_Filesystem.cpp"
    "${NX_ROOT_PATH}/source/NX_AudioClip.cpp"
    "${NX_ROOT_PATH}/source/NX_DataCodec.cpp"
    "${NX_ROOT_PATH}/source/NX_Codepoint.cpp"
    "${NX_ROOT_PATH}/source/NX_Clipboard.cpp"
    "${NX_ROOT_PATH}/source/NX_Material.cpp"
    "${NX_ROOT_PATH}/source/NX_Shader3D.cpp"
    "${NX_ROOT_PATH}/source/NX_Shader2D.cpp"
    "${NX_ROOT_PATH}/source/NX_Render3D.cpp"
    "${NX_ROOT_PATH}/source/NX_Render2D.cpp"
    "${NX_ROOT_PATH}/source/NX_Keyboard.cpp"
    "${NX_ROOT_PATH}/source/NX_Display.cpp"
    "${NX_ROOT_PATH}/source/NX_Cubemap.cpp"
    "${NX_ROOT_PATH}/source/NX_Texture.cpp"
    "${NX_ROOT_PATH}/source/NX_Runtime.cpp"
    "${NX_ROOT_PATH}/source/NX_Random.cpp"
    "${NX_ROOT_PATH}/source/NX_Camera.cpp"
    "${NX_ROOT_PATH}/source/NX_Memory.cpp"
    "${NX_ROOT_PATH}/source/NX_Window.cpp"
    "${NX_ROOT_PATH}/source/NX_Memory.cpp"
    "${NX_ROOT_PATH}/source/NX_Mouse.cpp"
    "${NX_ROOT_PATH}/source/NX_Image.cpp"
    "${NX_ROOT_PATH}/source/NX_Audio.cpp"
    "${NX_ROOT_PATH}/source/NX_Light.cpp"
    "${NX_ROOT_PATH}/source/NX_Model.cpp"
    "${NX_ROOT_PATH}/source/NX_Mesh.cpp"
    "${NX_ROOT_PATH}/source/NX_Font.cpp"
    "${NX_ROOT_PATH}/source/NX_Init.cpp"
    "${NX_ROOT_PATH}/source/NX_Math.cpp"
    "${NX_ROOT_PATH}/source/NX_Log.cpp"

)

# Create the Nexium library target

if(NX_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${NX_SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${NX_SOURCES})
endif()

# Link private deps to Nexium

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${NX_EXTERNAL_LIBS}
)

# Adding compile definitions for external sources

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        AL_LIBTYPE_STATIC
)

# Adding external sources to Nexium

target_sources(${PROJECT_NAME}
    PRIVATE
        "${NX_ROOT_PATH}/external/glad/src/gles2.c"
        "${NX_ROOT_PATH}/external/mojoAL/mojoal.c"
)

# Embed shaders

embed_assets(${PROJECT_NAME}
    "${NX_ROOT_PATH}/assets/brdf_lut_512_rg16_float.raw"
    "${NX_ROOT_PATH}/assets/font.ttf"
)

embed_shaders(${PROJECT_NAME}
    "${NX_ROOT_PATH}/shaders/common/screen.vert"
    "${NX_ROOT_PATH}/shaders/common/cube.vert"
    "${NX_ROOT_PATH}/shaders/process/cubemap_from_equirectangular.frag"
    "${NX_ROOT_PATH}/shaders/process/cubemap_irradiance.frag"
    "${NX_ROOT_PATH}/shaders/process/cubemap_prefilter.frag"
    "${NX_ROOT_PATH}/shaders/process/cubemap_skybox.frag"
    "${NX_ROOT_PATH}/shaders/process/downsampling.frag"
    "${NX_ROOT_PATH}/shaders/process/upsampling.frag"
    "${NX_ROOT_PATH}/shaders/process/ssao_bilateral_blur.frag"
    "${NX_ROOT_PATH}/shaders/process/ssao_pass.frag"
    "${NX_ROOT_PATH}/shaders/process/screen_quad.frag"
    "${NX_ROOT_PATH}/shaders/overlay/shape.vert"
    "${NX_ROOT_PATH}/shaders/overlay/shape.frag"
    "${NX_ROOT_PATH}/shaders/overlay/overlay.frag"
    "${NX_ROOT_PATH}/shaders/scene/light_culling.comp"
    "${NX_ROOT_PATH}/shaders/scene/skybox.vert"
    "${NX_ROOT_PATH}/shaders/scene/skybox.frag"
    "${NX_ROOT_PATH}/shaders/scene/scene_prepass.frag"
    "${NX_ROOT_PATH}/shaders/scene/scene.vert"
    "${NX_ROOT_PATH}/shaders/scene/scene_lit.frag"
    "${NX_ROOT_PATH}/shaders/scene/scene_unlit.frag"
    "${NX_ROOT_PATH}/shaders/scene/scene_shadow.frag"
    "${NX_ROOT_PATH}/shaders/scene/bloom_post.frag"
    "${NX_ROOT_PATH}/shaders/scene/ssao_post.frag"
    "${NX_ROOT_PATH}/shaders/scene/output.frag"
)

# Specify the include directories

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${NX_ROOT_PATH}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include"
        "${NX_ROOT_PATH}/external/freetype/include"
        "${NX_ROOT_PATH}/external/assimp/include"
        "${NX_ROOT_PATH}/external/FP16/include"
        "${NX_ROOT_PATH}/external/glad/include"
        "${NX_ROOT_PATH}/external/SDL/include"
        "${NX_ROOT_PATH}/external/physfs/src"
        "${NX_ROOT_PATH}/external/mojoAL/AL"
        "${NX_ROOT_PATH}/external/dr_libs"
        "${NX_ROOT_PATH}/external/stb"
)

# Link external dependencies

check_library_exists(m cos "" NX_HAVE_LIB_M)

if(NX_HAVE_LIB_M)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

# Plateform configuration

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NX_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NX_PLATFORM_MACOS)
elseif(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NX_PLATFORM_ANDROID)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NX_PLATFORM_LINUX)
endif()

# Build documentation

if(NX_BUILD_DOXYGEN)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Install configuration

if(NX_INSTALL)

    include(GNUInstallDirs)

    # Installing the library (static or shared)
    install(TARGETS ${PROJECT_NAME}
        EXPORT HyperionTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Installing vendored dependencies
    install(TARGETS ${NX_EXTERNAL_INSTALL_TARGETS}
        EXPORT HyperionTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Installing public headers
    install(DIRECTORY "${NX_ROOT_PATH}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )

    # Export for find_package
    install(EXPORT HyperionTargets
        FILE HyperionTargets.cmake
        NAMESPACE nexium::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nexium
    )

    # Minimal config for find_package
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/HyperionConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
        "${NX_ROOT_PATH}/cmake/HyperionConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/HyperionConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nexium
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/HyperionConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/HyperionConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nexium
    )

endif()

# Example configuration

if(NX_BUILD_TESTS)
    include("${NX_ROOT_PATH}/tests/CMakeLists.txt")
endif()
