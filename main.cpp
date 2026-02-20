#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QWindow>
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

    // --- YENİ EKLENEN KISIM: 2. EKRAN (PROJEKSİYON) YÖNETİMİ ---
    if (!engine.rootObjects().isEmpty()) {
        // QML'deki ana pencereyi (Window) C++ tarafında yakala
        QWindow *mainWindow = qobject_cast<QWindow*>(engine.rootObjects().first());

        if (mainWindow) {
            // Sisteme bağlı tüm ekranların listesini al
            QList<QScreen *> screens = QGuiApplication::screens();

            // Eğer 1'den fazla ekran (Yani Projeksiyon) bağlıysa:
            if (screens.count() > 1) {
                QScreen *projectorScreen = screens.at(1); // 0: Monitör, 1: Projeksiyon

                // Uygulamayı projeksiyona ata ve koordinatlarını oraya taşı
                mainWindow->setScreen(projectorScreen);
                mainWindow->setGeometry(projectorScreen->geometry());

                // İşletim sisteminin kafası karışmasın diye Tam Ekran modunu tekrar tetikle
                mainWindow->setVisibility(QWindow::FullScreen);

                qDebug() << "Sistem 2. ekranda (Projeksiyon) baslatildi:" << projectorScreen->name();
            } else {
                qDebug() << "Sadece 1 ekran bulundu, ana monitörde baslatiliyor.";
            }
        }
    }
    // -----------------------------------------------------------

    return app.exec();
}
