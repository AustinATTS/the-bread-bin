#include "gui/LoafRuntimeWidget.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace BreadBin::GUI {
LoafRuntimeWidget::LoafRuntimeWidget(QWidget* parent)
    : QWidget(parent), current_loaf_(nullptr) {
  SetupUI();
  ConnectSignals();

  status_timer_ = new QTimer(this);
  status_timer_->setInterval(2000);
  connect(status_timer_, &QTimer::timeout, this,
          &LoafRuntimeWidget::UpdateStatus);
}

LoafRuntimeWidget::~LoafRuntimeWidget() = default;

void LoafRuntimeWidget::SetupUI() {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(16, 16, 16, 16);
  main_layout->setSpacing(16);

  QGroupBox* info_group = new QGroupBox("▶️ Current Loaf", this);
  QVBoxLayout* info_layout = new QVBoxLayout(info_group);
  info_layout->setContentsMargins(12, 20, 12, 12);
  info_layout->setSpacing(12);

  loaf_name_label_ = new QLabel("No loaf loaded", this);
  QFont font = loaf_name_label_->font();
  font.setPointSize(16);
  font.setBold(true);
  loaf_name_label_->setFont(font);
  loaf_name_label_->setStyleSheet("color: #5d4e37; padding: 8px;");
  info_layout->addWidget(loaf_name_label_);

  status_label_ = new QLabel("Status: Stopped", this);
  status_label_->setStyleSheet(
      "font-size: 13px; color: #8b7a5e; padding: 4px;");
  info_layout->addWidget(status_label_);

  main_layout->addWidget(info_group);

  QGroupBox* items_group = new QGroupBox("📊 Item Status", this);
  QVBoxLayout* items_layout = new QVBoxLayout(items_group);
  items_layout->setContentsMargins(12, 20, 12, 12);

  item_status_list_ = new QListWidget(this);
  item_status_list_->setMinimumHeight(250);
  items_layout->addWidget(item_status_list_);

  main_layout->addWidget(items_group, 1);

  QHBoxLayout* button_layout = new QHBoxLayout();
  button_layout->setSpacing(12);

  run_button_ = new QPushButton("▶️ Run Loaf", this);
  run_button_->setMinimumHeight(44);
  run_button_->setMinimumWidth(140);
  run_button_->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #90c890, stop:1 #70b870);
                color: #1f3f1f;
                font-weight: 700;
                font-size: 14px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #80b880, stop:1 #60a860);
            }
            QPushButton:disabled {
                background-color: #e8dcc8;
                color: #a89580;
            }
        )");

  stop_button_ = new QPushButton("⏹️ Stop Loaf", this);
  stop_button_->setMinimumHeight(44);
  stop_button_->setMinimumWidth(140);
  stop_button_->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #e88888, stop:1 #d87070);
                color: #4f1f1f;
                font-weight: 700;
                font-size: 14px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #d87878, stop:1 #c86060);
            }
            QPushButton:disabled {
                background-color: #e8dcc8;
                color: #a89580;
            }
        )");
  stop_button_->setEnabled(false);

  refresh_button_ = new QPushButton("🔄 Refresh", this);
  refresh_button_->setMinimumHeight(44);
  refresh_button_->setMinimumWidth(120);

  button_layout->addWidget(run_button_);
  button_layout->addWidget(stop_button_);
  button_layout->addWidget(refresh_button_);
  button_layout->addStretch();

  main_layout->addLayout(button_layout);

  setLayout(main_layout);
}

void LoafRuntimeWidget::ConnectSignals() {
  connect(run_button_, &QPushButton::clicked, this,
          &LoafRuntimeWidget::OnRunLoaf);
  connect(stop_button_, &QPushButton::clicked, this,
          &LoafRuntimeWidget::OnStopLoaf);
  connect(refresh_button_, &QPushButton::clicked, this,
          &LoafRuntimeWidget::OnRefreshStatus);
}

void LoafRuntimeWidget::SetLoaf(std::shared_ptr<Loaf> loaf) {
  current_loaf_ = loaf;

  if (current_loaf_) {
    loaf_name_label_->setText(QString::fromStdString(current_loaf_->GetName()));
    RefreshLoafStatus();
  } else {
    loaf_name_label_->setText("No loaf loaded");
    status_label_->setText("Status: Stopped");
    item_status_list_->clear();
  }
}

void LoafRuntimeWidget::OnRunLoaf() {
  if (current_loaf_) {
    if (current_loaf_->Run()) {
      status_label_->setText("Status: Running");
      run_button_->setEnabled(false);
      stop_button_->setEnabled(true);
      status_timer_->start();
      emit loafStarted();
    } else {
      status_label_->setText("Status: Failed to start");
    }
  }
}

void LoafRuntimeWidget::OnStopLoaf() {
  if (current_loaf_) {
    if (current_loaf_->Stop()) {
      status_label_->setText("Status: Stopped");
      run_button_->setEnabled(true);
      stop_button_->setEnabled(false);
      status_timer_->stop();
      emit loafStopped();
    }
  }
}

void LoafRuntimeWidget::OnRefreshStatus() { RefreshLoafStatus(); }

void LoafRuntimeWidget::UpdateStatus() { RefreshLoafStatus(); }

void LoafRuntimeWidget::RefreshLoafStatus() {
  if (!current_loaf_) return;

  item_status_list_->clear();

  const auto& items = current_loaf_->GetItems();
  for (const auto& item : items) {
    QString status = QString::fromStdString(item->GetName());
    if (current_loaf_->IsRunning()) {
      status += " - Running";
    } else {
      status += " - Idle";
    }
    item_status_list_->addItem(status);
  }

  if (current_loaf_->IsRunning()) {
    status_label_->setText("Status: Running");
    run_button_->setEnabled(false);
    stop_button_->setEnabled(true);
  } else {
    status_label_->setText("Status: Stopped");
    run_button_->setEnabled(true);
    stop_button_->setEnabled(false);
  }
}
}  // namespace BreadBin::GUI