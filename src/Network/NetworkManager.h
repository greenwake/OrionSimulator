#pragma once
#include <QObject>
#include <QThread>

// Forward Declaration
class NetworkService;

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(quint16 port, QObject *parent = nullptr);
    ~NetworkManager();

signals:
    void rawInputReceived(float x, float y);

private:
    QThread *workerThread;
    NetworkService *worker;
};