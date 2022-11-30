This is a fork of `QtWebSockets` Qt-module.
* obtained with `git clone -b 5.15 https://code.qt.io/cgit/qt/qtwebsockets.git`
* renamed to `QtWebSocketsMod` with Search'n'Replace.
* A new method `QWebSocket::peerCertificateChain()` has been added to access `QSslSocket::peerCertificateChain()` that is hidden under the hood of `QWebSocketPrivate` implementation in case `QWebSocketServer::SecureMode` is used.

Make sure you have `qtbase5-private-dev` package installed.

Following is the original (except for module name) ReadMe:

---
### Introduction
`QtWebSocketsMod` is a pure Qt implementation of WebSockets - both client and server.
It is implemented as a Qt add-on module, that can easily be embedded into existing Qt projects. It has no other dependencies than Qt.

### Features
* Client and server capable
* Text and binary sockets
* Frame-based and message-based signals
* Strict Unicode checking
* WSS and proxy support

### Requirements
Qt 5.x

### Build And Usage
Checkout the source code from code.qt.io
Go into the source directory and execute:

    qmake
    make
    make install


The last command will install `QtWebSocketsMod` as a Qt module.

To use, add `websockets` to the QT variable.

`QT += websockets`

### Compliance
`QtWebSocketsMod` is compliant with [RFC6455](http://datatracker.ietf.org/doc/rfc6455/?include_text=1) and has been tested with the [Autobahn Testsuite](http://autobahn.ws/testsuite).

### Missing Features
* Extensions and sub-protocols
