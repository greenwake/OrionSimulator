#include "NetworkService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

NetworkService::NetworkService(quint16 dataPort, quint16 controlPort, QObject *parent)
    : QObject(parent), m_dataPort(dataPort), m_controlPort(controlPort) {}

void NetworkService::startServer() {
    // 1. ATIS PORTU (Örn: 12345)
    m_dataServer = new QTcpServer(this);
    connect(m_dataServer, &QTcpServer::newConnection, this, &NetworkService::handleDataConnection);
    if (!m_dataServer->listen(QHostAddress::Any, m_dataPort)) {
        qCritical() << "Data Server hatasi:" << m_dataServer->errorString();
    } else { qDebug() << "Data Server (Atis) dinleniyor Port:" << m_dataPort; }

    // 2. KONTROL PORTU (Örn: 12346)
    m_controlServer = new QTcpServer(this);
    connect(m_controlServer, &QTcpServer::newConnection, this, &NetworkService::handleControlConnection);
    if (!m_controlServer->listen(QHostAddress::Any, m_controlPort)) {
        qCritical() << "Control Server hatasi:" << m_controlServer->errorString();
    } else { qDebug() << "Control Server (Ayar) dinleniyor Port:" << m_controlPort; }
}

void NetworkService::handleDataConnection() {
    QTcpSocket *socket = m_dataServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [socket, this](){
        parseData(socket->readAll());
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void NetworkService::handleControlConnection() {
    QTcpSocket *socket = m_controlServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [socket, this](){
        parseControl(socket->readAll());
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void NetworkService::parseData(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("x") && obj.contains("y")) {
            emit inputReceived(obj["x"].toDouble(), obj["y"].toDouble());
        }
    }
}

void NetworkService::parseControl(const QByteArray &data) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) return;

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        int count = 0;
        QString p1 = "1. OYUNCU";
        QString p2 = "2. OYUNCU";

        if (obj.contains("player1name")) {
            p1 = obj["player1name"].toString();
            count = 1;
        }
        if (obj.contains("player2name")) {
            p2 = obj["player2name"].toString();
            count = 2; // Eğer player2name mesajda varsa otomatik 2 oyuncuya geçer
        }

        if (count > 0) {
            qDebug() << "TCP'den Yeni Oyuncu Ayari Alindi:" << count << p1 << p2;
            emit playerConfigReceived(count, p1, p2);
        }
    }
}
