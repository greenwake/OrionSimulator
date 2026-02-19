#pragma once
#include <QObject>
#include <QPointF>

class IScene : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sceneName READ sceneName CONSTANT)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)

public:
    explicit IScene(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IScene() {}

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void processShot(QPointF normalizedPos) = 0;

    QString sceneName() const { return m_sceneName; }
    int score() const { return m_score; }

signals:
    void scoreChanged(int newScore);
    void targetHit(float x, float y, bool isHit);

protected:
    QString m_sceneName;
    int m_score = 0;
    
    void setScore(int s) { 
        m_score = s; 
        emit scoreChanged(m_score); 
    }
};