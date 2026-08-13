cmake_minimum_required(VERSION 3.16)  # 为了 qt_generate_deploy_app_script 需要 3.14+，这里使用 3.16
project(CMAKE_K03 VERSION 1.0.0)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 Qt 的自动处理（moc, uic, rcc）
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

# 处理编译器输出乱码问题
if(MSVC)
    # 设置语言为英文
    add_compile_options(/diagnostics:caret)
    add_compile_options(/utf-8)
endif()

# Windows Unicode（MoreSet.h 等使用 taskschd.h/Lmcons.h 等 Windows SDK 头，qmake 项目默认定义）
add_definitions(-DUNICODE -D_UNICODE)

# 日志上下文（与 Project_K03.pro 的 DEFINES += QT_MESSAGELOGCONTEXT 对齐：qDebug 输出文件:行号）
add_definitions(-DQT_MESSAGELOGCONTEXT)


# 源文件（使用 GLOB 收集，修改后需手动重新运行 CMake）
# 注意：资源目录是 resources/（复数），res.qrc 内嵌皮肤图片与 .qm 翻译文件
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
    "src/*.ui"
    "resources/*.qrc"
)


# 如果没有找到任何源文件，则使用默认的 main.cpp
if(NOT PROJECT_SOURCES)
    set(PROJECT_SOURCES main.cpp)
endif()

# 强制指定 Qt 路径（VS CMake 默认 x64，需明确指向 32-bit Qt）
set(CMAKE_PREFIX_PATH "D:/Qt/5.15.2/msvc2019" ${CMAKE_PREFIX_PATH})

# 提示：Qt 5.15.2 msvc2019 为 32 位，请在 VS 中切换到 x86 配置
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Detected x64 toolchain — this project targets 32-bit Qt 5.15.2. Switch VS to x86-Debug.")
endif()

# 查找 Qt6，如果找不到则尝试 Qt5
# 组件对照 Project_K03.pro 的 QT += 列表：core gui concurrent svg axcontainer webenginewidgets
#   network multimedia multimediawidgets widgets printsupport（另加 Sql，代码有 QSqlDatabase 使用）
# 组件说明：Multimedia/QMultimediaWidgets(QMediaPlayer/QAudioInput/QVideoWidget)、
#           Concurrent(QtConcurrent)、WebEngineWidgets(微信扫码 WeChatQRCode)
# SerialPort 已移除：代码零使用，.pro 亦未声明
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Sql Network Multimedia MultimediaWidgets Concurrent Svg AxContainer PrintSupport WebEngineWidgets)

# 根据 Qt 版本设置部署脚本所需的组件
if(${QT_VERSION_MAJOR} EQUAL 6)
    # Qt6 需要 LinguistTools 用于翻译（可选）
    find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
    set(QT_LINGUIST_TOOLS Qt6::LinguistTools)
else()
    # Qt5 也支持 LinguistTools
    find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
    set(QT_LINGUIST_TOOLS Qt5::LinguistTools)
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# 应用图标 + 版本信息（app.rc.in → configure_file 注入绝对路径，MSVC/MinGW 均可用）
if(WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
endif()

# VS 调试时从 out/ 启动，确保能找到同目录的 SSL DLL
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out"
)

# 分层目录的 include 根路径（所有 #include 相对 src/ 即可）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party_library/spdlog/include
)


# 链接三方 FFMPEG（src/FeedBackC/ffmpage 使用 libavcodec/libavformat/libavutil/libswresample/libswscale）
set(FFMPEG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party_library/ffmpeg-n5.1-latest-win32-gpl-shared-5.1)
if(EXISTS ${FFMPEG_ROOT}/include AND EXISTS ${FFMPEG_ROOT}/lib)
    # 添加 ffmpeg 头文件路径
    target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_ROOT}/include)
    # 添加 ffmpeg 库路径
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_ROOT}/lib)
    # 链接 ffmpeg 库（仅链接代码实际使用的 5 个库）
    target_link_libraries(${PROJECT_NAME}
        avcodec
        avformat
        avutil
        swresample
        swscale
    )
    # 构建后把 ffmpeg 运行库 DLL 全部拷贝到输出目录（与 exe 同目录）
    file(GLOB FFMPEG_DLLS ${FFMPEG_ROOT}/bin/*.dll)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
else()
    message(WARNING "FFMPEG 目录不存在：${FFMPEG_ROOT}，将跳过 ffmpeg 链接")
endif()

# 链接 LibVLC（src/SpeakerListen.h 使用 <vlc/vlc.h>；libvlc.lib/libvlccore.lib 为官方 SDK 导入库）
set(VLC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party_library/vlc)
if(EXISTS ${VLC_ROOT}/include AND EXISTS ${VLC_ROOT}/lib)
    # 添加 vlc 头文件路径（含 vlc/ 子目录，代码中 #include <vlc/vlc.h>）
    target_include_directories(${PROJECT_NAME} PRIVATE ${VLC_ROOT}/include)
    # 添加 vlc 库路径
    target_link_directories(${PROJECT_NAME} PRIVATE ${VLC_ROOT}/lib)
    # 链接 libvlc 导入库
    target_link_libraries(${PROJECT_NAME}
        libvlc
        libvlccore
    )
    # 构建后把 VLC 运行库 DLL 拷贝到输出目录（与 exe 同目录）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${VLC_ROOT}/libvlc.dll
            ${VLC_ROOT}/libvlccore.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
else()
    message(WARNING "LibVLC 目录不存在：${VLC_ROOT}，将跳过 VLC 链接（SpeakerListen.h 需要 vlc/vlc.h）")
endif()

# 拷贝 DLL/ 运行库到输出目录（LoadLib.cpp/LoadApoDLL.cpp 按 exe 同目录动态加载
# ActHID/hidapi/DeviceUsb/LHDCX_XIBERIA_DLL/SaviUIControl/libcrypto/cacert.pem 等；
# ffmpeg/vlc 的 DLL 已由上方各自 POST_BUILD 处理，DLL/ 补齐其余运行库）
file(GLOB K03_RUNTIME_DLLS ${CMAKE_CURRENT_SOURCE_DIR}/DLL/*.dll)
list(APPEND K03_RUNTIME_DLLS
    ${CMAKE_CURRENT_SOURCE_DIR}/DLL/cacert.pem
    ${CMAKE_CURRENT_SOURCE_DIR}/DLL/LHDCX_XIBERIA_DLL.debug
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${K03_RUNTIME_DLLS}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
# 递归拷贝 DLL/Resources（main.cpp 按 exe 同目录加载 Resources/Font/*.otf 字体
# 与 Resources/images/systemBackground 背景图——漏拷则字体/背景不加载）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/DLL/Resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources
)

# 链接 Qt 库 + 系统库（ole32/uuid/version/opengl32/glu32 与 Project_K03.pro 的 LIBS 对齐）
target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::AxContainer
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    ole32
    uuid
    version
    opengl32
    glu32
)

# 添加翻译文件（如果存在 .ts 文件）
# 注意：qt_add_translations 是 Qt6 的 API，Qt5 需用 qt5_create_translation
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/translations)
    set(TS_FILES)
    file(GLOB TS_FILES "translations/*.ts")
    if(TS_FILES)
        if(${QT_VERSION_MAJOR} EQUAL 6)
            qt_add_translations(${PROJECT_NAME}
                TS_FILES ${TS_FILES}
                QM_FILES_OUTPUT_VARIABLE qm_files
            )
            target_sources(${PROJECT_NAME} PRIVATE ${qm_files})
        else()
            # 注意：不能用 qt5_create_translation（它生成的 lupdate 命令需要源文件列表，
            # 只传 .ts 时列表文件为空 → 构建报 "List file '' is not readable"）
            # qt5_add_translation 只做 lrelease（.ts → .qm），无需源文件
            qt5_add_translation(QM_FILES ${TS_FILES})
            target_sources(${PROJECT_NAME} PRIVATE ${QM_FILES})
        endif()
    endif()
endif()

# 设置安装和部署（仅当 Qt6 且不是交叉编译时自动执行）
if(${QT_VERSION_MAJOR} EQUAL 6 AND NOT CMAKE_CROSSCOMPILING)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME}
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
endif()

# 打印构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Qt version: ${QT_VERSION}")

# GUI 子系统（与 Project_K03.pro 的 /SUBSYSTEM:WINDOWS,"5.01" 对齐：无控制台窗口）
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

