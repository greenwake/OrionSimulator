#pragma once
#include <QObject>
#include <QThread>

// Forward Declaration
class NetworkService;

class NetworkManager : public QObject {
    Q_OBJECT
public:
    // public: bölümüne controlPort'u ekleyin
    explicit NetworkManager(quint16 dataPort, quint16 controlPort, QObject *parent = nullptr);
    ~NetworkManager();

signals:
    void rawInputReceived(float x, float y);
    // signals: bölümüne şunu ekleyin
    void playerConfigReceived(int playerCount, QString p1Name, QString p2Name);
private:
    QThread *workerThread;
    NetworkService *worker;
};
