#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QMenu>

namespace BreadBin {
    namespace GUI {

        struct LoafInfo {
            QString filepath;
            QString name;
            QDateTime lastOpened;
            bool isFavorite;
            QString category;
        };

        class HomeWidget : public QWidget {
            Q_OBJECT

            public:
                explicit HomeWidget(QWidget *parent = nullptr);
                ~HomeWidget();

                void refresh();

                void addRecentLoaf(const QString& filepath, const QString& name);

            signals:
                void openLoafRequested(const QString& filepath);
                void openLoafInTextEditorRequested(const QString& filepath);
                void newLoafRequested();

            private slots:
                void onRecentLoafDoubleClicked(QListWidgetItem* item);
                void onFavoriteLoafDoubleClicked(QListWidgetItem* item);
                void onCategoryLoafDoubleClicked(QListWidgetItem* item);
                void showContextMenu(const QPoint& pos);

            private:
                void setupUI();
                void connectSignals();
                void loadSettings();
                void saveSettings();
                void updateRecentList();
                void updateFavoritesList();
                void updateCategoriesList();
                void toggleFavorite(const QString& filepath);
                void setCategory(const QString& filepath, const QString& category);
                QString getSettingsFile() const;

                QListWidget *recentList_;
                QListWidget *favoritesList_;
                QListWidget *categoriesList_;
                QLabel *statusLabel_;

                QList<LoafInfo> recentLoafs_;
                QMap<QString, LoafInfo> allLoafs_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // HOMEWIDGET_H
