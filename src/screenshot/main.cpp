/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QDBusConnection>
#include <QtWidgets/QApplication>
#include <QtQuickControls2/QQuickStyle>

#include "screenshotclient.h"

#define TR(x) QT_TRANSLATE_NOOP("Command line parser", QStringLiteral(x))

int main(int argc, char *argv[])
{
    // HighDpi scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Setup application
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Screenshot"));
    app.setApplicationVersion(QStringLiteral(LIRISCREENSHOT_VERSION));
    app.setOrganizationDomain(QStringLiteral("liri.io"));
    app.setOrganizationName(QStringLiteral("Liri"));
    app.setDesktopFileName(QStringLiteral("io.liri.Screenshot.desktop"));

    // Set default style
    QQuickStyle::setStyle(QLatin1String("Material"));

    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Command line screencaster"));
    parser.addHelpOption();
    parser.addVersionOption();

    // Clipboard
    QCommandLineOption clipboardOption(QStringList() << QStringLiteral("c") << QStringLiteral("clipboard"),
                                       TR("Send the grab directly to the clipboard."));
    parser.addOption(clipboardOption);

    // Active window
    QCommandLineOption activeWindowOption(QStringList() << QStringLiteral("t") << QStringLiteral("active-window"),
                                          TR("Grab the active window instead of the entire screen."));
    parser.addOption(activeWindowOption);

    // Window
    QCommandLineOption windowOption(QStringList() << QStringLiteral("w") << QStringLiteral("window"),
                                    TR("Grab a window instead of the entire screen."));
    parser.addOption(windowOption);

    // Area
    QCommandLineOption areaOption(QStringList() << QStringLiteral("a") << QStringLiteral("area"),
                                  TR("Grab an area of the screen instead of the entire screen."));
    parser.addOption(areaOption);

    // Include pointer
    QCommandLineOption pointerOption(QStringList() << QStringLiteral("p") << QStringLiteral("include-pointer"),
                                     TR("Include the pointer with the screenshot."));
    parser.addOption(pointerOption);

    // Include border
    QCommandLineOption borderOption(QStringList() << QStringLiteral("b") << QStringLiteral("include-border"),
                                    TR("Include the window border with the screenshot."));
    parser.addOption(borderOption);

    // Don't include border
    QCommandLineOption noBorderOption(QStringList() << QStringLiteral("B") << QStringLiteral("remove-border"),
                                      TR("Do not include the window border with the screenshot."));
    parser.addOption(noBorderOption);

    // Delay
    QCommandLineOption delayOption(QStringList() << QStringLiteral("d") << QStringLiteral("delay"),
                                   TR("Take screenshot after specified delay."), TR("seconds"));
    parser.addOption(delayOption);

    // Interactive
    QCommandLineOption interactiveOption(QStringList() << QStringLiteral("i") << QStringLiteral("interactive"),
                                         TR("Interactively set options."));
    parser.addOption(interactiveOption);

    // File name
    QCommandLineOption filenameOption(QStringList() << QStringLiteral("f") << QStringLiteral("filename"),
                                      TR("Save screenshot directly to this file."), TR("filename"));
    parser.addOption(filenameOption);

    // Parse command line
    parser.process(app);

    // Check if the D-Bus session bus is available
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.");
        return 1;
    }

    // Run the application
    ScreenshotClient *screenshooter = new ScreenshotClient();
    if (parser.isSet(interactiveOption)) {
        QGuiApplication::postEvent(screenshooter, new InteractiveStartupEvent());
    } else {
        ScreenshotClient::What what = ScreenshotClient::Screens;
        if (parser.isSet(activeWindowOption))
            what = ScreenshotClient::ActiveWindow;
        else if (parser.isSet(windowOption))
            what = ScreenshotClient::Window;
        else if (parser.isSet(areaOption))
            what = ScreenshotClient::Area;

        ScreenshotClient::Effects effects = ScreenshotClient::NoEffect;
        if (parser.isSet(pointerOption))
            effects.setFlag(ScreenshotClient::OverlayCursorEffect);
        if (parser.isSet(borderOption))
            effects.setFlag(ScreenshotClient::KeepWindowBorderEffect);

        bool ok = false;
        int delay = parser.value(delayOption).toInt(&ok);
        if (!ok || delay < 1)
            delay = 0;

        QGuiApplication::postEvent(screenshooter, new StartupEvent(what, effects, delay));
    }
    QObject::connect(&app, &QGuiApplication::aboutToQuit,
                     screenshooter, &ScreenshotClient::deleteLater);

    return app.exec();
}
