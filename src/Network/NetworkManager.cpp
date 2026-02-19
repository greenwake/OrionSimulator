#include "NetworkManager.h"
#include "NetworkService.h"

NetworkManager::NetworkManager(quint16 dataPort, quint16 controlPort, QObject *parent) : QObject(parent) {
    workerThread = new QThread(this);
    worker = new NetworkService(dataPort, controlPort); // İki portu da gönderdik

    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &NetworkService::startServer);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(worker, &NetworkService::inputReceived, this, &NetworkManager::rawInputReceived);

    // YENİ EKLENDİ: Kontrol sinyalini dışarı aktar
    connect(worker, &NetworkService::playerConfigReceived, this, &NetworkManager::playerConfigReceived);

    workerThread->start();
}

NetworkManager::~NetworkManager() {
    workerThread->quit();
    workerThread->wait();
}
