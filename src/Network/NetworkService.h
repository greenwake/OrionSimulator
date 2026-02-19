#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkService : public QObject {
    Q_OBJECT
public:
    explicit NetworkService(quint16 dataPort, quint16 controlPort, QObject *parent = nullptr);

public slots:
    void startServer();
    void handleDataConnection();
    void handleControlConnection(); // YENİ: Kontrol portu için

signals:
    void inputReceived(float x, float y);
    void playerConfigReceived(int playerCount, QString p1Name, QString p2Name); // YENİ: JSON geldiğinde fırlatılır

private:
    void parseData(const QByteArray &data);
    void parseControl(const QByteArray &data); // YENİ: JSON ayırıcı

    QTcpServer *m_dataServer = nullptr;
    QTcpServer *m_controlServer = nullptr;

    quint16 m_dataPort;
    quint16 m_controlPort;
};
