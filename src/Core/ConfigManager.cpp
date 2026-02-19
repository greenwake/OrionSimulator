#include "ConfigManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager _instance;
    return _instance;
}

ConfigManager::ConfigManager() {}

void ConfigManager::loadConfig(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Config dosyasina erisilemedi:" << path;
        return;
    }
    m_config = QJsonDocument::fromJson(file.readAll()).object();
    qDebug() << "Konfigurasyon yuklendi.";
}

QVariant ConfigManager::getSetting(const QString& group, const QString& key, const QVariant& defaultValue) const {
    if (m_config.contains(group) && m_config[group].isObject()) {
        return m_config[group].toObject()[key].toVariant();
    }
    return defaultValue;
}