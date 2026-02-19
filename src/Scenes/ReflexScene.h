#pragma once
#include "IScene.h"
#include <QTimer>
#include <QMap>
#include <QVariantMap>
#include <QVariantList>
#include <QElapsedTimer>

struct ActiveTarget {
    int id;
    QPointF pos;
    float radius;
    QTimer* lifeTimer;
    int ownerId; // YENİ: Hangi oyuncunun hedefi olduğunu bilmek için
};

struct RoundStat {
    int level;
    int winnerId;
    int p1Score; int p2Score;
    int p1Shots; int p2Shots;
    int p1MaxCombo; int p2MaxCombo;
    qint64 durationMs;
};

class ReflexScene : public IScene {
    Q_OBJECT
public:
    explicit ReflexScene(int playerCount, QObject *parent = nullptr);
    ~ReflexScene() override;

    void start() override;
    void stop() override;
    void processShot(int playerId, QPointF normalizedPos) override;

private slots:
    void spawnTarget();
    void despawnTarget(int id, bool wasHit = false);
    void advanceLevel(); // YENİ: Seviye atlatma fonksiyonu

private:
    void checkLevelProgression(int playerId); // YENİ: 100 puana ulaşıldı mı kontrolü
    void generateReport();

    int m_playerCount;
    int m_p1Score = 0;
    int m_p2Score = 0;

    QTimer *m_spawnTimer;
    int m_targetIdCounter = 0;
    QMap<int, ActiveTarget> m_activeTargets;

    // --- OYUNLAŞTIRMA (GAMIFICATION) DEĞİŞKENLERİ ---
    int m_currentLevel = 1;
    int m_targetScore = 100;       // Sonraki seviye için gereken puan
    int m_spawnIntervalMs = 1000;  // Balonların çıkma sıklığı (Zorluk için değişecek)
    int m_targetLifeTimeMs = 3000; // Balonun ekranda kalma süresi (Zorluk için değişecek)
    bool m_isTransitioning = false;// Seviye geçişinde atışları engellemek için

    // --- YENİ EKLENEN GAMIFICATION DEĞİŞKENLERİ ---
    int m_p1Combo = 0;    int m_p2Combo = 0;
    int m_p1MaxCombo = 0; int m_p2MaxCombo = 0;
    int m_p1Shots = 0;    int m_p2Shots = 0;

    QElapsedTimer m_levelTimer; // Tur süresini ölçecek
    QList<RoundStat> m_roundStats; // Geçmiş turların raporları
};
