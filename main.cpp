#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Klasör yolları include_directories sayesinde gerekmez
#include "ConfigManager.h"
#include "NetworkManager.h"
#include "SceneManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1. Config Yükle
    ConfigManager::instance().loadConfig("config.json");
    int dataPort = ConfigManager::instance().getSetting("Network", "Port", 12345).toInt();
    int controlPort = ConfigManager::instance().getSetting("Network", "ControlPort", 12346).toInt(); // YENİ

    // 2. Manager'ları Başlat
    NetworkManager networkManager(dataPort, controlPort);
    SceneManager sceneManager;

    // 3. Sinyal Bağlantısı (TCP -> Scene)
    QObject::connect(&networkManager, &NetworkManager::rawInputReceived,
                     &sceneManager, &SceneManager::handleInput);

    // YENİ BAĞLANTI: NetworkManager'dan gelen oyuncu bilgisini SceneManager'a (oradan QML'e) ilet
    QObject::connect(&networkManager, &NetworkManager::playerConfigReceived,
                     &sceneManager, &SceneManager::handlePlayerConfig);
    // 4. QML Başlat
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("sceneManager", &sceneManager);

    const QUrl url(QStringLiteral("qrc:/Orion/src/UI/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
