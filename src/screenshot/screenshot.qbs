import qbs 1.0

QtGuiApplication {
    name: "liri-screenshot"
    targetName: "liri-screenshot"

    Depends { name: "lirideployment" }
    Depends { name: "Qt"; submodules: ["core", "dbus", "gui", "widgets", "quickcontrols2"]; versionAtLeast: project.minimumQtVersion }

    cpp.defines: [
        'LIRISCREENSHOT_VERSION="' + project.version + '"',
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII"
    ]

    cpp.includePaths: base.concat([product.buildDirectory])

    files: [
        "imageprovider.cpp",
        "imageprovider.h",
        "main.cpp",
        "screenshotclient.cpp",
        "screenshotclient.h",
        "screenshot.qrc",
    ]

    Group {
        name: "QML Files"
        files: ["*.qml"]
        prefix: "qml/"
        fileTags: ["qml"]
    }

    Group {
        name: "Translations"
        files: ["*_*.ts"]
        prefix: "translations/"
    }

    Group {
        name: "D-Bus Interfaces"
        files: [
            "io.liri.Shell.Screenshooter.xml",
        ]
        fileTags: ["qt.dbus.interface"]
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.binDir
        fileTagsFilter: "application"
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/liri-screenshot/translations"
        fileTagsFilter: "qm"
    }
}
