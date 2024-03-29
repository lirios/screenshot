# Translations
file(GLOB LiriScreenshot_TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/../../translations/*_*.ts")
qt5_add_translation(LiriScreenshot_QM_FILES ${LiriScreenshot_TRANSLATIONS})
install(FILES ${LiriScreenshot_QM_FILES}
        DESTINATION "${INSTALL_DATADIR}/liri-screenshot/translations")

if(NOT TARGET Liri::AuroraClient)
    find_package(Liri1AuroraClient REQUIRED)
endif()

liri_add_executable(LiriScreenshot
    OUTPUT_NAME
        "liri-screenshot"
    SOURCES
        imageprovider.cpp
        imageprovider.h
        main.cpp
        screenshotclient.cpp
        screenshotclient.h
        ${LiriScreenshot_QM_FILES}
    RESOURCES
        screenshot.qrc
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        LIRISCREENSHOT_VERSION="${PROJECT_VERSION}"
    DESKTOP
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.Screenshot.desktop"
    LIBRARIES
        Qt5::Core
        Qt5::Widgets
        Qt5::QuickControls2
        Liri::AuroraClient
)

liri_finalize_executable(LiriScreenshot)
