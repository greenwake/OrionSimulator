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

signals:
    void currentSceneChanged();
    void targetHit(float x, float y, bool isHit);

private:
    IScene *m_currentScene = nullptr;
};
