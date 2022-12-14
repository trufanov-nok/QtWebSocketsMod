/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSocketsMod module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmlwebsockets_plugin.h"

#include <QtQml>

#include "qqmlwebsocket.h"
#include "qqmlwebsocketserver.h"

QT_BEGIN_NAMESPACE

void QtWebSocketsModDeclarativeModule::registerTypes(const char *uri)
{
    // ### Qt 6: Remove support for the "Qt.WebSockets" QML module.
    Q_ASSERT(uri == QLatin1String("QtWebSocketsMod") || uri == QLatin1String("Qt.WebSockets"));

    // @uri QtWebSocketsMod
    qmlRegisterType<QQmlWebSocket>(uri, 1 /*major*/, 0 /*minor*/, "WebSocket");
    qmlRegisterType<QQmlWebSocket, 1>(uri, 1 /*major*/, 1 /*minor*/, "WebSocket");
    qmlRegisterType<QQmlWebSocketServer>(uri, 1 /*major*/, 0 /*minor*/, "WebSocketServer");

    // Auto-increment the import to stay in sync with ALL future QtQuick minor versions
    qmlRegisterModule(uri, 1, QT_VERSION_MINOR);
}

QT_END_NAMESPACE
