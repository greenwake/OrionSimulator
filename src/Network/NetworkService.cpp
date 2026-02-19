#include "NetworkService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

NetworkService::NetworkService(quint16 port, QObject *parent) 
    : QObject(parent), m_port(port) {}

void NetworkService::startServer() {
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &NetworkService::handleConnection);
    
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCritical() << "Server baslatilamadi:" << m_server->errorString();
    } else {
        qDebug() << "Server dinleniyor Port:" << m_port;
    }
}

void NetworkService::handleConnection() {
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [socket, this](){
        QByteArray data = socket->readAll();
        parseAndEmit(data);
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void NetworkService::parseAndEmit(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON Parse Hatasi:" << parseError.errorString();
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("x") && obj.contains("y")) {
            float x = static_cast<float>(obj["x"].toDouble());
            float y = static_cast<float>(obj["y"].toDouble());
            emit inputReceived(x, y);
            // Debug için konsola yazalım
            qDebug() << "Network Atisi:" << x << y;
        }
    }
}