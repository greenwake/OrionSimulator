#pragma once
#include <QObject>
#include <QJsonObject>
#include <QVariant>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();

    void loadConfig(const QString& path);

    QVariant getSetting(const QString& group, const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
    ConfigManager();
    QJsonObject m_config;
};
