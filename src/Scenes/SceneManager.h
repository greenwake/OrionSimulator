#pragma once
#include <QObject>

class IScene;

class SceneManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(IScene* currentScene READ currentScene NOTIFY currentSceneChanged)

public:
    explicit SceneManager(QObject *parent = nullptr); // Sadece tanım

    IScene* currentScene() const;

    Q_INVOKABLE void loadScene(const QString &sceneType);

public slots:
    void handleInput(float x, float y);

    void startGame(int playerCount, const QString &sceneType);
    void handlePlayerConfig(int playerCount, const QString &p1Name, const QString &p2Name);
signals:
    void currentSceneChanged();
    void targetHit(float x, float y, bool isHit);

    void scoreChanged(int playerId, int newScore);
    // QML'e iletilecek sinyaller
    void targetSpawned(int id, float x, float y, float size, QString color);
    void targetRemoved(int id, bool wasHit);

    // QML'e iletilecek sinyaller
    void levelChanged(int newLevel);
    void roundWinner(int playerId, QString message);

    void gameOver(QVariantMap reportData);

    void playerConfigUpdated(int playerCount, QString p1Name, QString p2Name);

private:
    IScene *m_currentScene = nullptr;
    int m_playerCount = 1;
    int m_p1Score = 0;
    int m_p2Score = 0;
};
