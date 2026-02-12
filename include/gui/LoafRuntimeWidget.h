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
                explicit LoafRuntimeWidget (QWidget *parent = nullptr);
                ~LoafRuntimeWidget ( );

                void SetLoaf (std::shared_ptr<Loaf> loaf);

            signals:
                void loafStarted();
                void loafStopped();

            private slots:
                void OnRunLoaf ( );
                void OnStopLoaf ( );
                void OnRefreshStatus ( );
                void UpdateStatus ( );

            private:
                void SetupUI ( );
                void ConnectSignals ( );
                void RefreshLoafStatus ( );

                std::shared_ptr<Loaf> current_loaf_;

                QLabel *loaf_name_label_;
                QLabel *status_label_;
                QListWidget *item_status_list_;

                QPushButton *run_button_;
                QPushButton *stop_button_;
                QPushButton *refresh_button_;

                QTimer *status_timer_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // LOAFRUNTIMEWIDGET_H
