#ifndef LOAFRUNTIMEWIDGET_H
#define LOAFRUNTIMEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <memory>
#include "Loaf.h"

namespace BreadBin {
    namespace GUI {

        class LoafRuntimeWidget : public QWidget {
            Q_OBJECT

            public:
                explicit LoafRuntimeWidget(QWidget *parent = nullptr);
                ~LoafRuntimeWidget();

                void setLoaf(std::shared_ptr<Loaf> loaf);

            signals:
                void loafStarted();
                void loafStopped();

            private slots:
                void onRunLoaf();
                void onStopLoaf();
                void onRefreshStatus();
                void updateStatus();

            private:
                void setupUI();
                void connectSignals();
                void refreshLoafStatus();

                std::shared_ptr<Loaf> currentLoaf_;

                QLabel *loafNameLabel_;
                QLabel *statusLabel_;
                QListWidget *itemStatusList_;

                QPushButton *runButton_;
                QPushButton *stopButton_;
                QPushButton *refreshButton_;

                QTimer *statusTimer_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // LOAFRUNTIMEWIDGET_H
