#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkService : public QObject {
    Q_OBJECT
public:
    explicit NetworkService(quint16 port, QObject *parent = nullptr);

public slots:
    void startServer();
    void handleConnection();

signals:
    void inputReceived(float x, float y);

private:
    void parseAndEmit(const QByteArray &data);
    QTcpServer *m_server = nullptr;
    quint16 m_port;
};