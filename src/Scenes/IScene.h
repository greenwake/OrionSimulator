#pragma once
#include <QObject>
#include <QPointF>
#include <QString>

class IScene : public QObject {
    Q_OBJECT

public:
    explicit IScene(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IScene() {}

    virtual void start() = 0;
    virtual void stop() = 0;

    // YENİ: Atışı yapan oyuncunun ID'si de geliyor
    virtual void processShot(int playerId, QPointF normalizedPos) = 0;

signals:
    // Sahnenin QML'e ve SceneManager'a göndereceği sinyaller
    void scoreUpdated(int playerId, int newScore);
    void targetSpawned(int id, float x, float y, float size, QString color);
    void targetRemoved(int id, bool wasHit);

    void levelChanged(int newLevel);
    void roundWinner(int playerId, QString message);

    void gameOver(QVariantMap reportData);
    void targetMoved(int id, float x, float y); // Hareket eden hedef için
};
