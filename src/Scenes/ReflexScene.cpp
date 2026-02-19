#include "ReflexScene.h"
#include <QRandomGenerator>
#include <QDebug>
#include <cmath> // sqrt ve pow için

ReflexScene::ReflexScene(int playerCount, QObject *parent)
    : IScene(parent), m_playerCount(playerCount)
{
    m_spawnTimer = new QTimer(this);
    connect(m_spawnTimer, &QTimer::timeout, this, &ReflexScene::spawnTarget);
}

ReflexScene::~ReflexScene() {
    stop();
}

void ReflexScene::start() {
    m_p1Score = 0; m_p2Score = 0;
    m_currentLevel = 1;
    m_spawnIntervalMs = 1000; m_targetLifeTimeMs = 3000;
    m_isTransitioning = false;
    m_roundStats.clear();

    // İstatistikleri Sıfırla
    m_p1Combo = 0; m_p2Combo = 0;
    m_p1MaxCombo = 0; m_p2MaxCombo = 0;
    m_p1Shots = 0; m_p2Shots = 0;

    emit scoreUpdated(1, 0);
    if (m_playerCount == 2) emit scoreUpdated(2, 0);
    emit levelChanged(m_currentLevel);

    m_levelTimer.start(); // Kronometreyi başlat
    m_spawnTimer->start(m_spawnIntervalMs);
}

void ReflexScene::stop() {

    m_spawnTimer->stop();

    for (int id : m_activeTargets.keys()) { despawnTarget(id); }
}



void ReflexScene::spawnTarget() {
    if(m_isTransitioning) return;

    for (int p = 1; p <= m_playerCount; ++p) {
        m_targetIdCounter++;
        int id = m_targetIdCounter;
        double randX = QRandomGenerator::global()->generateDouble();
        double randY = QRandomGenerator::global()->generateDouble();
        float rx = 0.0f; float ry = 0.1f + static_cast<float>(randY) * 0.8f;
        QString color = "red";

        if (m_playerCount == 1) {
            rx = 0.1f + static_cast<float>(randX) * 0.8f;
        } else {
            if (p == 1) { rx = 0.05f + static_cast<float>(randX) * 0.40f; color = "#00FF00"; }
            else { rx = 0.55f + static_cast<float>(randX) * 0.40f; color = "cyan"; }
        }

        float size = 150.0f;
        float normalizedRadius = (size / 1920.0f) / 2.0f;

        QTimer* lifeTimer = new QTimer(this);
        lifeTimer->setSingleShot(true);
        connect(lifeTimer, &QTimer::timeout, this, [this, id]() {
            despawnTarget(id, false); // Süre bittiğinde wasHit=false gider
        });
        lifeTimer->start(m_targetLifeTimeMs);

        // YENİ: Hangi oyuncunun balonu olduğunu (p) struct içine kaydediyoruz
        m_activeTargets.insert(id, {id, QPointF(rx, ry), normalizedRadius, lifeTimer, p});
        emit targetSpawned(id, rx, ry, size, color);
    }
}

void ReflexScene::despawnTarget(int id, bool wasHit) {
    if (!m_activeTargets.contains(id)) return;

    // YENİ: Eğer hedef VURULMADAN silindiyse (süresi dolduysa), o hedefin sahibinin KOMBOSUNU SIFIRLA!
    if (!wasHit) {
        int owner = m_activeTargets[id].ownerId;
        if (owner == 1) m_p1Combo = 0;
        else m_p2Combo = 0;
    }

    m_activeTargets[id].lifeTimer->stop();
    m_activeTargets[id].lifeTimer->deleteLater();
    m_activeTargets.remove(id);
    emit targetRemoved(id, wasHit);
}

void ReflexScene::processShot(int playerId, QPointF normalizedPos) {
    if (m_isTransitioning) return;

    // YENİ: Atış sayısını artır
    if (playerId == 1) m_p1Shots++; else m_p2Shots++;

    int hitTargetId = -1;
    for (const auto& target : m_activeTargets) {
        float aspect = 1920.0f / 1080.0f;
        float dx = (target.pos.x() - normalizedPos.x()) * aspect;
        float dy = target.pos.y() - normalizedPos.y();
        if (std::sqrt(dx * dx + dy * dy) <= target.radius * aspect) {
            hitTargetId = target.id;
            break;
        }
    }

    if (hitTargetId != -1) {
        // İSABET!
        despawnTarget(hitTargetId, true);

        // KOMBO VE PUAN HESAPLAMA
        int currentCombo = (playerId == 1) ? ++m_p1Combo : ++m_p2Combo;

        // Max kombo rekorunu güncelle
        if (playerId == 1 && currentCombo > m_p1MaxCombo) m_p1MaxCombo = currentCombo;
        if (playerId == 2 && currentCombo > m_p2MaxCombo) m_p2MaxCombo = currentCombo;

        // Puan Çarpanı
        int points = 5; // Temel puan
        if (currentCombo >= 10) points = 15; // 3X Puan
        else if (currentCombo >= 5) points = 10; // 2X Puan

        if (playerId == 1) {
            m_p1Score += points;
            emit scoreUpdated(1, m_p1Score);
        } else {
            m_p2Score += points;
            emit scoreUpdated(2, m_p2Score);
        }

        checkLevelProgression(playerId);
    }
    else {
        // KARAVANA (ISKA)! Komboyu sıfırla.
        if (playerId == 1) m_p1Combo = 0;
        else m_p2Combo = 0;
    }
}

void ReflexScene::checkLevelProgression(int playerId) {
    int currentScore = (playerId == 1) ? m_p1Score : m_p2Score;

    if (currentScore >= m_targetScore) {
        m_isTransitioning = true;
        m_spawnTimer->stop();

        // TUR BİLGİLERİNİ KAYDET
        RoundStat stat;
        stat.level = m_currentLevel;
        stat.winnerId = playerId;
        stat.p1Score = m_p1Score; stat.p2Score = m_p2Score;
        stat.p1Shots = m_p1Shots; stat.p2Shots = m_p2Shots;
        stat.p1MaxCombo = m_p1MaxCombo; stat.p2MaxCombo = m_p2MaxCombo;
        stat.durationMs = m_levelTimer.elapsed();
        m_roundStats.append(stat);

        // Kalan balonları sessizce (patlamadan) sil
        QList<int> keys = m_activeTargets.keys();
        for (int id : keys) { despawnTarget(id, false); }

        if (m_currentLevel == 8) {
            // OYUN KOMPLE BİTTİ
            emit roundWinner(playerId, QString("OYUN BİTTİ! KAZANAN: %1. OYUNCU").arg(playerId));
            QTimer::singleShot(3000, this, &ReflexScene::generateReport);
        } else {
            // DİĞER SEVİYEYE GEÇ
            emit roundWinner(playerId, QString("%1. OYUNCU %2. SEVİYEYİ KAZANDI!").arg(playerId).arg(m_currentLevel));
            QTimer::singleShot(3000, this, &ReflexScene::advanceLevel);
        }
    }
}

void ReflexScene::advanceLevel() {
    m_currentLevel++;

    // YENİ: Puanları ve istatistikleri sıfırla (Mevcut seviye için 100 puan kuralı)
    m_p1Score = 0; m_p2Score = 0;
    m_p1Combo = 0; m_p2Combo = 0;
    m_p1MaxCombo = 0; m_p2MaxCombo = 0;
    m_p1Shots = 0; m_p2Shots = 0;

    emit scoreUpdated(1, 0);
    if(m_playerCount == 2) emit scoreUpdated(2, 0);

    // Zorluk Eğrisi (Hızlanma)
    m_spawnIntervalMs = qMax(400, int(m_spawnIntervalMs * 0.85));
    m_targetLifeTimeMs = qMax(800, int(m_targetLifeTimeMs * 0.85));

    emit levelChanged(m_currentLevel);

    m_levelTimer.restart(); // Kronometreyi sıfırla
    m_isTransitioning = false;
    m_spawnTimer->start(m_spawnIntervalMs);
}

// YENİ: RAPOR HAZIRLAMA FONKSİYONU
void ReflexScene::generateReport() {
    QVariantMap report;
    report["playerCount"] = m_playerCount;

    QVariantList rounds;
    for (const auto& r : m_roundStats) {
        QVariantMap rm;
        rm["level"] = r.level;
        rm["winner"] = r.winnerId;
        rm["p1Score"] = r.p1Score; rm["p2Score"] = r.p2Score;
        rm["p1Shots"] = r.p1Shots; rm["p2Shots"] = r.p2Shots;
        rm["p1MaxCombo"] = r.p1MaxCombo; rm["p2MaxCombo"] = r.p2MaxCombo;
        // Süreyi saniyeye çevirip formatla
        rm["timeStr"] = QString::number(r.durationMs / 1000.0, 'f', 1) + " sn";
        rounds.append(rm);
    }

    report["rounds"] = rounds;
    emit gameOver(report); // QML'e yolla
}
