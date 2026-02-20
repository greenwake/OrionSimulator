#pragma once
#include "IScene.h"
#include <QTimer>
#include <QMap>
#include <QElapsedTimer>
#include <QVariantMap>

struct PolygonTarget {
    int id;
    int ownerId;
    float x;
    float y;
    float dx; // X eksenindeki hızı
    float dy; // Y eksenindeki hızı
    float radius;
    float size; // Ekranda çizilecek piksel boyutu
    QString imageSource;
};

struct PolygonRoundStat {
    int level;
    int winnerId;
    int p1Score; int p2Score;
    int p1Shots; int p2Shots;
    int p1MaxPoints; int p2MaxPoints; // Tek atışta alınan en yüksek puan (Örn: 10)
    qint64 durationMs;
};

class PolygonScene : public IScene {
    Q_OBJECT
public:
    explicit PolygonScene(int playerCount, QObject *parent = nullptr);
    ~PolygonScene() override;

    void start() override;
    void stop() override;
    void processShot(int playerId, QPointF normalizedPos) override;

private slots:
    void spawnTargetsForLevel();
    void updatePhysics(); // Hedeflerin hareketini hesaplayan döngü
    void advanceLevel();

private:
    void checkLevelProgression(int playerId);
    void generateReport();
    int calculateHitScore(float distance, float targetRadius); // Halka puanını hesaplar

    int m_playerCount;
    int m_p1Score = 0; int m_p2Score = 0;
    int m_p1Shots = 0; int m_p2Shots = 0;
    int m_p1MaxPoints = 0; int m_p2MaxPoints = 0;

    int m_currentLevel = 1;
    int m_targetScore = 100;
    bool m_isTransitioning = false;

    QTimer *m_physicsTimer; // 60 FPS hareket motoru
    int m_targetIdCounter = 0;
    QMap<int, PolygonTarget> m_activeTargets;

    QElapsedTimer m_levelTimer;
    QList<PolygonRoundStat> m_roundStats;
};
