import qbs 1.0

QtGuiApplication {
    name: "liri-screenshot"
    targetName: "liri-screenshot"

    Depends { name: "lirideployment" }
    Depends { name: "Qt"; submodules: ["core", "dbus", "gui", "widgets", "quickcontrols2"]; versionAtLeast: project.minimumQtVersion }
    Depends { name: "LiriTranslations" }

    cpp.defines: [
        'LIRISCREENSHOT_VERSION="' + project.version + '"',
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII"
    ]

    cpp.includePaths: base.concat([product.buildDirectory])

    Qt.core.resourcePrefix: "/"
    Qt.core.resourceSourceBase: sourceDirectory

    files: [
        "imageprovider.cpp",
        "imageprovider.h",
        "main.cpp",
        "screenshotclient.cpp",
        "screenshotclient.h",
    ]

    Group {
        name: "Resource Data"
        prefix: "qml/"
        files: [
            "main.qml",
        ]
        fileTags: ["qt.core.resource_data"]
    }

    Group {
        name: "Desktop File"
        files: ["io.liri.Screenshot.desktop.in"]
        fileTags: ["liri.desktop.template"]
    }

    Group {
        name: "Desktop File Translations"
        files: ["io.liri.Screenshot_*.desktop"]
        prefix: "translations/"
        fileTags: ["liri.desktop.translations"]
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
        qbs.installDir: lirideployment.applicationsDir
        fileTagsFilter: "liri.desktop.file"
    }

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.dataDir + "/liri-screenshot/translations"
        fileTagsFilter: "qm"
    }
}
