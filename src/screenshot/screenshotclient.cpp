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

#include <QDateTime>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QQmlContext>
#include <QStandardPaths>
#include <QTimer>

#include "screenshotclient.h"
#include "screenshooter_interface.h"

/*
 * InteractiveStartupEvent
 */

static const QEvent::Type InteractiveStartupEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

InteractiveStartupEvent::InteractiveStartupEvent()
    : QEvent(InteractiveStartupEventType)
{
}

/*
 * StartupEvent
 */

static const QEvent::Type StartupEventType =
        static_cast<QEvent::Type>(QEvent::registerEventType());

StartupEvent::StartupEvent(ScreenshotClient::What w, ScreenshotClient::Effects e, int d)
    : QEvent(StartupEventType)
    , what(w)
    , effects(e)
    , delay(d)
{
}

/*
 * Screenshooter
 */

ScreenshotClient::ScreenshotClient(QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlApplicationEngine(this))
    , m_imageProvider(new ImageProvider())
{
}

bool ScreenshotClient::isEnabled() const
{
    return m_interface != nullptr;
}

bool ScreenshotClient::event(QEvent *event)
{
    if (event->type() == InteractiveStartupEventType) {
        m_interactive = true;
        initialize();
        return true;
    } else if (event->type() == StartupEventType) {
        StartupEvent *e = static_cast<StartupEvent *>(event);
        m_cliOptions.what = e->what;
        m_cliOptions.pointer = e->effects.testFlag(OverlayCursorEffect);
        m_cliOptions.border = e->effects.testFlag(KeepWindowBorderEffect);
        m_cliOptions.delay = e->delay;
        initialize();
        return true;
    }

    return QObject::event(event);
}

void ScreenshotClient::initialize()
{
    Q_ASSERT(!m_initialized);

    const QString service = QLatin1String("io.liri.Session");
    const QString path = QLatin1String("/Screenshot");
    const QDBusConnection bus = QDBusConnection::sessionBus();
    m_interface = new IoLiriShellScreenshooterInterface(service, path, bus);
    if (!m_interface->isValid()) {
        qWarning("Cannot find D-Bus service, please run this program under Liri Shell.");
        m_interface->deleteLater();
        m_interface = nullptr;
        QCoreApplication::exit(1);
        return;
    }

    m_initialized = true;

    if (m_interactive) {
        m_engine->addImageProvider(QLatin1String("screenshooter"), m_imageProvider);
        m_engine->rootContext()->setContextProperty(QLatin1String("Screenshooter"), this);
        m_engine->load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    } else {
        QTimer::singleShot(m_cliOptions.delay * 1000, this, [this] {
            takeScreenshot(m_cliOptions.what, m_cliOptions.pointer, m_cliOptions.border);
        });
    }

    Q_EMIT enabledChanged();
}

void ScreenshotClient::takeScreenshot(What what, bool includePointer, bool includeBorder)
{
    if (!isEnabled()) {
        qWarning("Cannot take screenshots until we are bound to the interfaces");
        return;
    }

    if (m_inProgress) {
        qWarning("Cannot take another screenshot while a previous capture is in progress");
        return;
    }

    QDBusPendingCallWatcher *watcher = nullptr;

    switch (what) {
    case Screens: {
        QDBusPendingCall pendingCall = m_interface->captureScreens(includePointer);
        watcher = new QDBusPendingCallWatcher(pendingCall, this);
        break;
    }
    case ActiveWindow: {
        QDBusPendingCall pendingCall = m_interface->captureActiveWindow(includePointer, includeBorder);
        watcher = new QDBusPendingCallWatcher(pendingCall, this);
        break;
    }
    case Window: {
        QDBusPendingCall pendingCall = m_interface->captureWindow(includePointer, includeBorder);
        watcher = new QDBusPendingCallWatcher(pendingCall, this);
        break;
    }
    case Area: {
        QDBusPendingCall pendingCall = m_interface->captureArea();
        watcher = new QDBusPendingCallWatcher(pendingCall, this);
        break;
    }
    default:
        break;
    }

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QByteArray> reply = *self;

        self->deleteLater();

        if (reply.isError()) {
            qWarning("Failed to capture the screen: %s", reply.error().message().toUtf8().constData());
            QGuiApplication::exit(1);
        }

        QByteArray data = reply.argumentAt<0>();
        m_imageProvider->image.loadFromData(data, "PNG");

        Q_EMIT screenshotDone();

        m_inProgress = false;

        // Quit in non-interactive mode because we only take one screenshot
        if (!m_interactive)
            QGuiApplication::quit();
    });

    m_inProgress = true;
}

QString ScreenshotClient::screenshotFileName() const
{
    return QStringLiteral("%1/%2.png")
            .arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                 tr("Screenshot from %1").arg(QDateTime::currentDateTime().toString(QLatin1String("yyyy-MM-dd hh:mm:ss"))));
}

void ScreenshotClient::saveScreenshot(const QUrl &fileName)
{
    m_imageProvider->image.save(fileName.toLocalFile());
}
