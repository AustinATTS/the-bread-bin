#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QString>
#include <QMap>

namespace BreadBin::GUI::Resources {
    QString LoadText (const QString& resource_path);
    QString ApplyTemplate (const QString& resource_path, const QMap<QString, QString>& values);
} // namespace BreadBin::GUI::Resources

#endif // RESOURCELOADER_H