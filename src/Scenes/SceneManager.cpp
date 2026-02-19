#include "SceneManager.h"
#include "IScene.h"
#include <QDebug>
#include <QPointF>
#include "ReflexScene.h"

SceneManager::SceneManager(QObject *parent) : QObject(parent) {}

IScene* SceneManager::currentScene() const {
    return m_currentScene;
}

void SceneManager::loadScene(const QString &sceneType) {
    // İleride buraya Factory eklenecek
    qDebug() << "Sahne yükleme istendi:" << sceneType;
}
void SceneManager::startGame(int playerCount, const QString &sceneType) {
    m_playerCount = playerCount;

    // Eski sahneyi temizle
    if (m_currentScene) {
        m_currentScene->stop();
        m_currentScene->deleteLater();
        m_currentScene = nullptr;
    }

    // YENİ SAHNEYİ OLUŞTUR
    if (sceneType == "Refleks") {
        m_currentScene = new ReflexScene(m_playerCount, this);

        // Sahnenin sinyallerini (Oyun Motorunu) QML arayüzüne (SceneManager'a) bağla
        connect(m_currentScene, &IScene::scoreUpdated, this, &SceneManager::scoreChanged);
        connect(m_currentScene, &IScene::targetSpawned, this, &SceneManager::targetSpawned);
        connect(m_currentScene, &IScene::targetRemoved, this, &SceneManager::targetRemoved);

        connect(m_currentScene, &IScene::levelChanged, this, &SceneManager::levelChanged);
        connect(m_currentScene, &IScene::roundWinner, this, &SceneManager::roundWinner);

        connect(m_currentScene, &IScene::gameOver, this, &SceneManager::gameOver);

        m_currentScene->start();
    }
}

void SceneManager::handleInput(float x, float y) {
    int playerId = 1;
    if (m_playerCount == 2 && x >= 0.5f) {
        playerId = 2;
    }

    emit targetHit(x, y, true); // Vuruş efekti (sarı nokta) çıksın

    // Atışı sahneye gönder (Matematiği Scene halledecek)
    if (m_currentScene) {
        m_currentScene->processShot(playerId, QPointF(x, y));
    }
}

void SceneManager::handlePlayerConfig(int playerCount, const QString &p1Name, const QString &p2Name) {
    // TCP'den gelen bilgiyi anında QML arayüzüne iletiyoruz
    emit playerConfigUpdated(playerCount, p1Name, p2Name);
}
