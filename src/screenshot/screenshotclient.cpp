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
#include <QPainter>
#include <QQmlContext>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

#include "screenshotclient.h"

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
    , m_screencopy(new WlrScreencopyManagerV1())
{
    connect(m_screencopy, &WlrScreencopyManagerV1::activeChanged, this, [this] {
        bool oldValue = m_enabled;

        if (!oldValue && m_screencopy->isActive() && m_initialized)
            m_enabled = true;
        else if (oldValue && (!m_screencopy->isActive() || !m_initialized))
            m_enabled = false;

        if (oldValue != m_enabled)
            emit enabledChanged();
    });
}

bool ScreenshotClient::isEnabled() const
{
    return m_enabled;
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
        m_cliOptions.effects = e->effects;
        m_cliOptions.delay = e->delay;
        initialize();
        return true;
    }

    return QObject::event(event);
}

void ScreenshotClient::initialize()
{
    Q_ASSERT(!m_initialized);

    m_initialized = true;

    if (m_interactive) {
        m_engine->addImageProvider(QLatin1String("screenshooter"), m_imageProvider);
        m_engine->rootContext()->setContextProperty(QLatin1String("Screenshooter"), this);
        m_engine->load(QUrl(QLatin1String("qrc:/qml/main.qml")));
    } else {
        QTimer::singleShot(m_cliOptions.delay * 1000, this, [this] {
            takeScreenshot(m_cliOptions.what,
                           m_cliOptions.effects.testFlag(OverlayCursorEffect),
                           m_cliOptions.effects.testFlag(KeepWindowBorderEffect));
        });
    }

    if (m_screencopy->isActive()) {
        m_enabled = true;
        emit enabledChanged();
    }
}

void ScreenshotClient::done()
{
    emit screenshotDone();

    m_inProgress = false;

    // Quit in non-interactive mode because we only take one screenshot
    if (!m_interactive)
        QGuiApplication::quit();
}

void ScreenshotClient::handleFrameCopied(const QImage &image)
{
    auto *frame = qobject_cast<WlrScreencopyFrameV1 *>(sender());
    if (!frame)
        return;

    QPainter painter(&m_finalImage);
    painter.drawImage(frame->screen()->geometry().topLeft(), image);

    m_screensToGo--;

    if (m_screensToGo == 0) {
        m_imageProvider->image = m_finalImage;
        done();
    }
}

void ScreenshotClient::takeScreenshot(What what, bool includePointer, bool includeBorder)
{
    Q_UNUSED(includeBorder)

    if (!isEnabled()) {
        qWarning("Cannot take screenshots until ready");
        return;
    }

    if (m_inProgress) {
        qWarning("Cannot take another screenshot while a previous capture is in progress");
        return;
    }

    switch (what) {
    case Screens: {
        const auto screens = qGuiApp->screens();
        m_screensToGo = screens.size();

        QRect screensGeometry;
        for (auto *screen : screens)
            screensGeometry |= screen->geometry();
        m_finalImage = QImage(screensGeometry.size(), QImage::Format_RGB32);

        for (auto *screen : screens) {
            auto *frame = m_screencopy->captureScreen(screen, includePointer);
            connect(frame, &WlrScreencopyFrameV1::copied, this, &ScreenshotClient::handleFrameCopied);
        }
        break;
    }
    default:
        break;
    }

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
    if (m_imageProvider->image.save(fileName.toLocalFile()))
        qInfo("Screenshot saved to \"%s\" successfully", fileName.toLocalFile().toUtf8().constData());
    else
        qWarning("Failed to save screenshot to \"%s\"", fileName.toLocalFile().toUtf8().constData());
}
