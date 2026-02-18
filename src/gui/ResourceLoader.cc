#include "gui/ResourceLoader.h"
#include <QFile>

namespace BreadBin::GUI::Resources {
    QString LoadText (const QString& resource_path) {
        QFile file(resource_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

    QString ApplyTemplate (const QString& resource_path, const QMap<QString, QString>& values) {
        QString result = LoadText(resource_path);
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            result.replace("{{" + it.key() + "}}", it.value());
        }
        return result;
    }
} // namespace BreadBin::GUI::Resources