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
            QDateTime last_opened;
            bool is_favorite;
            QString category;
        };

        class HomeWidget : public QWidget {
            Q_OBJECT

            public:
                explicit HomeWidget (QWidget *parent = nullptr);
                ~HomeWidget ( );

                void Refresh ( );

                void AddRecentLoaf (const QString& filepath, const QString& name);

            signals:
                void OpenLoafRequested (const QString& filepath);
                void OpenLoafInTextEditorRequested (const QString& filepath);
                void NewLoafRequested ( );

            private slots:
                void OnRecentLoafDoubleClicked (QListWidgetItem* item);
                void OnFavoriteLoafDoubleClicked (QListWidgetItem* item);
                void OnCategoryLoafDoubleClicked (QListWidgetItem* item);
                void ShowContextMenu (const QPoint& pos);

            private:
                void SetupUI ( );
                void ConnectSignals ( );
                void LoadSettings ( );
                void SaveSettings ( );
                void UpdateRecentList ( );
                void UpdateFavoritesList ( );
                void UpdateCategoriesList ( );
                void ToggleFavorite (const QString& filepath);
                void SetCategory (const QString& filepath, const QString& category);
                QString GetSettingsFile (  ) const;

                QListWidget *recent_list_;
                QListWidget *favorites_list_;
                QListWidget *categories_list_;
                QLabel *status_label_;

                QList<LoafInfo> recent_loafs_;
                QMap<QString, LoafInfo> all_loafs_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // HOMEWIDGET_H
