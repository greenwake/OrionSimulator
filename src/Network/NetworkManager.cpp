#include "NetworkManager.h"
#include "NetworkService.h"

NetworkManager::NetworkManager(quint16 port, QObject *parent) : QObject(parent) {
    workerThread = new QThread(this);
    worker = new NetworkService(port); // Parent verme, moveToThread yapacağız
    
    worker->moveToThread(workerThread);

    // Thread sinyalleri
    connect(workerThread, &QThread::started, worker, &NetworkService::startServer);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    
    // Veri sinyali (Worker -> Manager)
    connect(worker, &NetworkService::inputReceived, this, &NetworkManager::rawInputReceived);

    workerThread->start();
}

NetworkManager::~NetworkManager() {
    workerThread->quit();
    workerThread->wait();
}