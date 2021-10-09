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

#ifndef SCREENSHOTCLIENT_H
#define SCREENSHOTCLIENT_H

#include <QEvent>
#include <QFile>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QQmlApplicationEngine>

#include <LiriWaylandClient/WlrScreencopyManagerV1>

#include "imageprovider.h"

class ScreenshotClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
public:
    enum What {
        Screens = 1,
        ActiveWindow,
        Window,
        Area
    };
    Q_ENUM(What)

    enum Effect {
        NoEffect = 0x0,
        OverlayCursorEffect = 0x1,
        KeepWindowBorderEffect = 0x2
    };
    Q_ENUM(Effect)
    Q_DECLARE_FLAGS(Effects, Effect)

    explicit ScreenshotClient(QObject *parent = nullptr);

    bool isEnabled() const;

    Q_INVOKABLE void takeScreenshot(What what, bool includePointer, bool includeBorder);
    Q_INVOKABLE QString screenshotFileName() const;
    Q_INVOKABLE void saveScreenshot(const QUrl &fileName);

Q_SIGNALS:
    void enabledChanged();
    void screenshotDone();

protected:
    bool event(QEvent *event) override;

private:
    bool m_initialized = false;
    bool m_enabled = false;
    bool m_interactive = false;
    bool m_inProgress = false;
    int m_screensToGo = 0;
    QImage m_finalImage;
    QQmlApplicationEngine *m_engine = nullptr;
    ImageProvider *m_imageProvider = nullptr;
    WlrScreencopyManagerV1 *m_screencopy = nullptr;

    struct {
        What what;
        Effects effects;
        int delay;
    } m_cliOptions;

    void initialize();
    void done();

private Q_SLOTS:
    void handleFrameCopied(const QImage &image);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ScreenshotClient::Effects)

class InteractiveStartupEvent : public QEvent
{
public:
    InteractiveStartupEvent();
};

class StartupEvent : public QEvent
{
public:
    StartupEvent(ScreenshotClient::What what, ScreenshotClient::Effects effects, int delay);

    ScreenshotClient::What what;
    ScreenshotClient::Effects effects;
    int delay;
};

#endif // SCREENSHOTCLIENT_H
