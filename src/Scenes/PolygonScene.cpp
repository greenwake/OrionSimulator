#include "PolygonScene.h"
#include <QRandomGenerator>
#include <QDebug>
#include <cmath>

PolygonScene::PolygonScene(int playerCount, QObject *parent)
    : IScene(parent), m_playerCount(playerCount)
{
    // 60 FPS (16ms) hızında çalışan fizik motoru (Hareketli hedefler için)
    m_physicsTimer = new QTimer(this);
    connect(m_physicsTimer, &QTimer::timeout, this, &PolygonScene::updatePhysics);
}

PolygonScene::~PolygonScene() { stop(); }

void PolygonScene::start() {
    m_p1Score = 0; m_p2Score = 0;
    m_currentLevel = 1;
    m_isTransitioning = false;
    m_roundStats.clear();

    m_p1Shots = 0; m_p2Shots = 0;
    m_p1MaxPoints = 0; m_p2MaxPoints = 0;

    emit scoreUpdated(1, 0);
    if (m_playerCount == 2) emit scoreUpdated(2, 0);
    emit levelChanged(m_currentLevel);

    m_levelTimer.start();
    spawnTargetsForLevel();
}

void PolygonScene::stop() {
    m_physicsTimer->stop();
    m_activeTargets.clear();
}

void PolygonScene::spawnTargetsForLevel() {
    m_activeTargets.clear();

    // SEVİYE PARAMETRELERİ
    float targetSize = 300.0f; // Varsayılan büyük boyut (Level 1 & 3)
    float baseSpeedX = 0.0f;
    float baseSpeedY = 0.0f;

    if (m_currentLevel == 2) {
        targetSize = 150.0f; // Level 2: Uzakta (Küçük)
    } else if (m_currentLevel == 3) {
        targetSize = 300.0f; // Level 3: Yakında ama hareketli (1X hız)
        baseSpeedX = 0.003f; baseSpeedY = 0.002f;
    } else if (m_currentLevel == 4) {
        targetSize = 150.0f; // Level 4: Uzakta ve hareketli (1.5X hız)
        baseSpeedX = 0.0045f; baseSpeedY = 0.003f;
    } else if (m_currentLevel == 5) {
        targetSize = 150.0f; // Level 5: Uzakta ve çok hızlı (2X hız)
        baseSpeedX = 0.006f; baseSpeedY = 0.004f;
    }

    float normalizedRadius = (targetSize / 1920.0f) / 2.0f;

    // Her oyuncu için 1 adet KALICI hedef oluştur
    for (int p = 1; p <= m_playerCount; ++p) {
        m_targetIdCounter++;

        // Rastgele başlangıç pozisyonu (Kulvarın tam ortalarına yakın)
        float startX = (p == 1) ? 0.25f : 0.75f;
        if (m_playerCount == 1) startX = 0.5f;
        float startY = 0.5f;

        // Farklı hedefler eklemeye müsait yapı

        QString imgSource = "qrc:/Orion/assets/images/target_classic.png";

        PolygonTarget target = {
            m_targetIdCounter, p,
            startX, startY,
            baseSpeedX, baseSpeedY,
            normalizedRadius, targetSize, imgSource
        };

        // Eğer hareketli seviyeyse yönleri rastgele belirle
        if (baseSpeedX > 0) {
            if (QRandomGenerator::global()->generate() % 2 == 0) target.dx = -target.dx;
            if (QRandomGenerator::global()->generate() % 2 == 0) target.dy = -target.dy;
        }

        m_activeTargets.insert(target.id, target);

        // QML'e rengi değil, görselin yolunu (imageSource) gönderiyoruz
        emit targetSpawned(target.id, target.x, target.y, target.size, imgSource);
    }

    // Hareket varsa fizik motorunu çalıştır
    if (m_currentLevel >= 3) m_physicsTimer->start(16);
    else m_physicsTimer->stop();
}

void PolygonScene::updatePhysics() {
    if (m_isTransitioning) return;

    for (auto &t : m_activeTargets) {
        t.x += t.dx;
        t.y += t.dy;

        // KULVAR SINIRLARI (Çarpıp dönmesi için)
        float minX = 0.05f; float maxX = 0.95f;

        if (m_playerCount == 2) {
            if (t.ownerId == 1) { maxX = 0.48f; } // 1. Oyuncu Sol Taraf
            else { minX = 0.52f; }                // 2. Oyuncu Sağ Taraf
        }

        float minY = 0.15f; float maxY = 0.85f; // Alt üst sınırlar

        // Duvara çarpma kontrolü ve sekme (Bounce)
        if (t.x - t.radius <= minX) { t.x = minX + t.radius; t.dx = std::abs(t.dx); }
        if (t.x + t.radius >= maxX) { t.x = maxX - t.radius; t.dx = -std::abs(t.dx); }
        if (t.y - t.radius <= minY) { t.y = minY + t.radius; t.dy = std::abs(t.dy); }
        if (t.y + t.radius >= maxY) { t.y = maxY - t.radius; t.dy = -std::abs(t.dy); }

        // QML'e yeni konumu bildir
        emit targetMoved(t.id, t.x, t.y);
    }
}

int PolygonScene::calculateHitScore(float distance, float targetRadius) {
    // Hedefin yarıçapını 6 eşit halkaya bölüyoruz (Görselinizdeki gibi: 10, 9, 8, 7, 6, 5)
    float ringWidth = targetRadius / 6.0f;

    if (distance <= ringWidth * 1) return 10; // Tam Onikiden (Bullseye)
    if (distance <= ringWidth * 2) return 9;
    if (distance <= ringWidth * 3) return 8;
    if (distance <= ringWidth * 4) return 7;
    if (distance <= ringWidth * 5) return 6;
    if (distance <= ringWidth * 6) return 5;  // En dış halka
    return 0; // Karavana
}

void PolygonScene::processShot(int playerId, QPointF normalizedPos) {
    if (m_isTransitioning) return;

    if (playerId == 1) m_p1Shots++; else m_p2Shots++;

    for (const auto& t : m_activeTargets) {
        // Sadece kendi kulvarındaki hedefe ateş edebilir
        if (t.ownerId != playerId) continue;

        float aspect = 1920.0f / 1080.0f;
        float dx = (t.x - normalizedPos.x()) * aspect;
        float dy = t.y - normalizedPos.y();
        float distance = std::sqrt(dx * dx + dy * dy);

        // VURULDU MU?
        if (distance <= t.radius * aspect) {
            // Hangi halkadan vurduğunu hesapla
            int points = calculateHitScore(distance, t.radius * aspect);

            // Rekor puanı kaydet (Örn: İlk defa 10 vurduysa rapora yansısın)
            if (playerId == 1 && points > m_p1MaxPoints) m_p1MaxPoints = points;
            if (playerId == 2 && points > m_p2MaxPoints) m_p2MaxPoints = points;

            // Puanı Ekle
            if (playerId == 1) {
                m_p1Score += points;
                emit scoreUpdated(1, m_p1Score);
            } else {
                m_p2Score += points;
                emit scoreUpdated(2, m_p2Score);
            }

            checkLevelProgression(playerId);
            break; // Bir mermi aynı anda iki hedefi vuramaz
        }
    }
}

void PolygonScene::checkLevelProgression(int playerId) {
    int currentScore = (playerId == 1) ? m_p1Score : m_p2Score;

    if (currentScore >= m_targetScore) {
        m_isTransitioning = true;
        m_physicsTimer->stop();

        // Rapor Verilerini Kaydet
        PolygonRoundStat stat;
        stat.level = m_currentLevel;
        stat.winnerId = playerId;
        stat.p1Score = m_p1Score; stat.p2Score = m_p2Score;
        stat.p1Shots = m_p1Shots; stat.p2Shots = m_p2Shots;
        stat.p1MaxPoints = m_p1MaxPoints; stat.p2MaxPoints = m_p2MaxPoints;
        stat.durationMs = m_levelTimer.elapsed();
        m_roundStats.append(stat);

        // Mevcut hedefleri ekrandan sil
        QList<int> keys = m_activeTargets.keys();
        for (int id : keys) { emit targetRemoved(id, false); }
        m_activeTargets.clear();

        if (m_currentLevel == 5) {
            emit roundWinner(playerId, QString("OYUN BİTTİ! KAZANAN: %1. OYUNCU").arg(playerId));
            QTimer::singleShot(3000, this, &PolygonScene::generateReport);
        } else {
            emit roundWinner(playerId, QString("%1. OYUNCU %2. SEVİYEYİ KAZANDI!").arg(playerId).arg(m_currentLevel));
            QTimer::singleShot(3000, this, &PolygonScene::advanceLevel);
        }
    }
}

void PolygonScene::advanceLevel() {
    m_currentLevel++;
    m_p1Score = 0; m_p2Score = 0;
    m_p1Shots = 0; m_p2Shots = 0;
    m_p1MaxPoints = 0; m_p2MaxPoints = 0;

    emit scoreUpdated(1, 0);
    if(m_playerCount == 2) emit scoreUpdated(2, 0);
    emit levelChanged(m_currentLevel);

    m_levelTimer.restart();
    m_isTransitioning = false;
    spawnTargetsForLevel();
}

void PolygonScene::generateReport() {
    QVariantMap report;
    report["playerCount"] = m_playerCount;

    QVariantList rounds;
    for (const auto& r : m_roundStats) {
        QVariantMap rm;
        rm["level"] = r.level;
        rm["winner"] = r.winnerId;
        rm["p1Score"] = r.p1Score; rm["p2Score"] = r.p2Score;
        rm["p1Shots"] = r.p1Shots; rm["p2Shots"] = r.p2Shots;

        // Refleks'teki "Max Kombo" sütununu, Poligon'da "Max Puan (Örn: 10 Puan)" olarak kullanacağız
        rm["p1MaxCombo"] = QString("%1 P").arg(r.p1MaxPoints);
        rm["p2MaxCombo"] = QString("%1 P").arg(r.p2MaxPoints);

        rm["timeStr"] = QString::number(r.durationMs / 1000.0, 'f', 1) + " sn";
        rounds.append(rm);
    }

    report["rounds"] = rounds;
    emit gameOver(report);
}
