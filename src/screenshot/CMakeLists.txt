# Translations
file(GLOB LiriScreenshot_TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*_*.ts")
qt5_add_translation(LiriScreenshot_QM_FILES ${LiriScreenshot_TRANSLATIONS})
install(FILES ${LiriScreenshot_QM_FILES}
        DESTINATION "${INSTALL_DATADIR}/liri-screenshot/translations")

liri_translate_desktop(_desktop_file SOURCES "io.liri.Screenshot.desktop.in")

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
        "${_desktop_file}"
    DBUS_INTERFACE_SOURCES
        io.liri.Shell.Screenshooter.xml
    LIBRARIES
        Qt5::Core
        Qt5::DBus
        Qt5::Widgets
        Qt5::QuickControls2
)
