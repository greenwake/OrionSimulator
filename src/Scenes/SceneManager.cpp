#include "SceneManager.h"
#include "IScene.h"
#include <QDebug>
#include <QPointF>

SceneManager::SceneManager(QObject *parent) : QObject(parent) {}

IScene* SceneManager::currentScene() const {
    return m_currentScene;
}

void SceneManager::loadScene(const QString &sceneType) {
    // İleride buraya Factory eklenecek
    qDebug() << "Sahne yükleme istendi:" << sceneType;
}

void SceneManager::handleInput(float x, float y) {
    // Bypass / Test Modu
    // Gelen veriyi direkt QML'e yansıtıyoruz ki sarı noktayı görelim.
    emit targetHit(x, y, true);

    // Eğer aktif bir sahne varsa ona da bildir
    if (m_currentScene) {
        m_currentScene->processShot(QPointF(x, y));
    }
}