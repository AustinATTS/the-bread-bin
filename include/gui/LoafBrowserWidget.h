#ifndef LOAFBROWSERWIDGET_H
#define LOAFBROWSERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <vector>

namespace BreadBin {
    namespace GUI {

        struct LoafFileInfo {
            QString filepath;
            QString name;
            QString description;
            int itemCount;
            QString lastModified;
        };

        class LoafBrowserWidget : public QWidget {
            Q_OBJECT

            public:
                explicit LoafBrowserWidget(QWidget *parent = nullptr);
                ~LoafBrowserWidget();

                void refreshLoafFiles();

                void setSearchPaths(const QStringList& paths);

            signals:
                void loafSelected(const QString& filepath);
                void loafOpened(const QString& filepath);

            private slots:
                void onRefreshClicked();
                void onSearchChanged(const QString& text);
                void onLoafSelected();
                void onOpenClicked();
                void onDeleteClicked();

            private:
                void setupUI();
                void connectSignals();
                void updateLoafList();
                void updateLoafPreview(const LoafFileInfo& info);
                void scanForLoafFiles();
                LoafFileInfo loadLoafInfo(const QString& filepath);

                std::vector<LoafFileInfo> loafFiles_;
                std::vector<LoafFileInfo> filteredFiles_;
                QStringList searchPaths_;

                QLineEdit *searchEdit_;
                QListWidget *fileList_;
                QTextEdit *previewText_;
                QPushButton *refreshButton_;
                QPushButton *openButton_;
                QPushButton *deleteButton_;
                QLabel *statusLabel_;
                QLabel *pathsLabel_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // LOAFBROWSERWIDGET_H
