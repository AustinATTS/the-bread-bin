#include "gui/LoafBrowserWidget.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <fstream>
#include <sstream>

#include "Loaf.h"

namespace BreadBin::GUI {
LoafBrowserWidget::LoafBrowserWidget(QWidget* parent)
    : QWidget(parent), paths_label_(nullptr) {
  QStringList default_paths;
  default_paths << QDir::homePath() + "/.breadbin/loafs";
  default_paths << QStandardPaths::writableLocation(
                       QStandardPaths::DocumentsLocation) +
                       "/BreadBin/loafs";
  default_paths << QDir::currentPath();
  SetSearchPaths(default_paths);

  SetupUI();
  ConnectSignals();
  RefreshLoafFiles();
}

LoafBrowserWidget::~LoafBrowserWidget() {}

void LoafBrowserWidget::SetupUI() {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(16, 16, 16, 16);
  main_layout->setSpacing(16);

  QGroupBox* search_group = new QGroupBox("🔍 Search", this);
  QHBoxLayout* search_layout = new QHBoxLayout(search_group);
  search_layout->setContentsMargins(12, 20, 12, 12);
  search_layout->setSpacing(12);

  QLabel* search_label = new QLabel("Filter:", this);
  search_label->setStyleSheet(
      "font-weight: 600; font-size: 13px; min-width: 60px;");
  search_edit_ = new QLineEdit(this);
  search_edit_->setPlaceholderText("Search loaf files...");
  search_edit_->setMinimumHeight(36);
  search_layout->addWidget(search_label);
  search_layout->addWidget(search_edit_, 1);

  main_layout->addWidget(search_group);

  QHBoxLayout* content_layout = new QHBoxLayout();
  content_layout->setSpacing(16);

  QGroupBox* list_group = new QGroupBox("📁 Loaf Files", this);
  list_group->setMinimumWidth(300);
  QVBoxLayout* list_layout = new QVBoxLayout(list_group);
  list_layout->setContentsMargins(12, 20, 12, 12);
  list_layout->setSpacing(12);

  file_list_ = new QListWidget(this);
  file_list_->setMinimumHeight(250);
  list_layout->addWidget(file_list_, 1);

  QHBoxLayout* button_layout = new QHBoxLayout();
  button_layout->setSpacing(8);
  refresh_button_ = new QPushButton("🔄 Refresh", this);
  refresh_button_->setMinimumHeight(40);
  refresh_button_->setMinimumWidth(100);
  open_button_ = new QPushButton("📂 Open", this);
  open_button_->setMinimumHeight(40);
  open_button_->setMinimumWidth(100);
  open_button_->setEnabled(false);
  delete_button_ = new QPushButton("🗑️ Delete", this);
  delete_button_->setMinimumHeight(40);
  delete_button_->setMinimumWidth(100);
  delete_button_->setEnabled(false);
  button_layout->addWidget(refresh_button_);
  button_layout->addWidget(open_button_);
  button_layout->addWidget(delete_button_);
  button_layout->addStretch();
  list_layout->addLayout(button_layout);

  content_layout->addWidget(list_group, 1);

  QGroupBox* preview_group = new QGroupBox("👁️ Preview", this);
  preview_group->setMinimumWidth(350);
  QVBoxLayout* preview_layout = new QVBoxLayout(preview_group);
  preview_layout->setContentsMargins(12, 20, 12, 12);

  preview_text_ = new QTextEdit(this);
  preview_text_->setReadOnly(true);
  preview_text_->setMinimumHeight(250);
  preview_text_->setStyleSheet(R"(
            QTextEdit {
                background-color: #fafafa;
                border: 2px solid #e8dcc8;
                border-radius: 6px;
                padding: 12px;
                color: #2d1f0f;
                font-size: 13px;
            }
        )");
  preview_layout->addWidget(preview_text_, 1);

  content_layout->addWidget(preview_group, 1);

  main_layout->addLayout(content_layout, 1);

  // Search path hint with modern styling
  paths_label_ = new QLabel(this);
  paths_label_->setWordWrap(true);
  paths_label_->setText("Watching folders: " + search_paths_.join(" | "));
  paths_label_->setStyleSheet(
      "color: #8b7a5e; font-size: 11px; padding: 4px; font-style: italic;");
  main_layout->addWidget(paths_label_);

  // Status with modern styling
  status_label_ = new QLabel("Ready", this);
  status_label_->setStyleSheet(
      "color: #8b7a5e; font-size: 12px; padding: 8px;");
  main_layout->addWidget(status_label_);

  setLayout(main_layout);
}

void LoafBrowserWidget::ConnectSignals() {
  connect(refresh_button_, &QPushButton::clicked, this,
          &LoafBrowserWidget::OnRefreshClicked);
  connect(search_edit_, &QLineEdit::textChanged, this,
          &LoafBrowserWidget::OnSearchChanged);
  connect(file_list_, &QListWidget::itemSelectionChanged, this,
          &LoafBrowserWidget::OnLoafSelected);
  connect(file_list_, &QListWidget::itemDoubleClicked,
          [this](QListWidgetItem*) { OnOpenClicked(); });
  connect(open_button_, &QPushButton::clicked, this,
          &LoafBrowserWidget::OnOpenClicked);
  connect(delete_button_, &QPushButton::clicked, this,
          &LoafBrowserWidget::OnDeleteClicked);
}

void LoafBrowserWidget::SetSearchPaths(const QStringList& paths) {
  search_paths_ = paths;
  if (paths_label_) {
    paths_label_->setText("Watching folders: " + search_paths_.join(" | "));
  }
}

void LoafBrowserWidget::RefreshLoafFiles() {
  ScanForLoafFiles();
  UpdateLoafList();
}

void LoafBrowserWidget::ScanForLoafFiles() {
  loaf_files_.clear();

  for (const QString& path : search_paths_) {
    QDir dir(path);
    if (!dir.exists()) {
      continue;
    }

    QStringList filters;
    filters << "*.loaf";

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : files) {
      LoafFileInfo info = LoadLoafInfo(fileInfo.absoluteFilePath());
      loaf_files_.push_back(info);
    }
  }

  status_label_->setText(
      QString("Found %1 loaf files").arg(loaf_files_.size()));
}

LoafFileInfo LoafBrowserWidget::LoadLoafInfo(const QString& filepath) {
  LoafFileInfo info;
  info.filepath = filepath;

  QFileInfo fileInfo(filepath);
  info.lastModified = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss");

  BreadBin::Loaf loaf;
  if (loaf.Load(filepath.toStdString())) {
    info.name = QString::fromStdString(loaf.GetName());
    info.description = QString::fromStdString(loaf.GetDescription());
    info.itemCount = static_cast<int>(loaf.GetItems().size());
  } else {
    info.itemCount = 0;
  }

  if (info.name.isEmpty()) {
    info.name = fileInfo.baseName();
  }

  return info;
}

void LoafBrowserWidget::OnRefreshClicked() { RefreshLoafFiles(); }

void LoafBrowserWidget::OnSearchChanged(const QString& text) {
  UpdateLoafList();
}

void LoafBrowserWidget::OnLoafSelected() {
  QListWidgetItem* item = file_list_->currentItem();
  if (!item) {
    open_button_->setEnabled(false);
    delete_button_->setEnabled(false);
    preview_text_->clear();
    return;
  }

  open_button_->setEnabled(true);
  delete_button_->setEnabled(true);

  int index = file_list_->currentRow();
  if (index >= 0 && index < filtered_files_.size()) {
    UpdateLoafPreview(filtered_files_[index]);
    emit LoafSelected(filtered_files_[index].filepath);
  }
}

void LoafBrowserWidget::OnOpenClicked() {
  QListWidgetItem* item = file_list_->currentItem();
  if (!item) {
    return;
  }

  int index = file_list_->currentRow();
  if (index >= 0 && index < filtered_files_.size()) {
    emit LoafOpened(filtered_files_[index].filepath);
    status_label_->setText("Opened: " + filtered_files_[index].name);
  }
}

void LoafBrowserWidget::OnDeleteClicked() {
  QListWidgetItem* item = file_list_->currentItem();
  if (!item) {
    return;
  }

  int index = file_list_->currentRow();
  if (index >= 0 && index < filtered_files_.size()) {
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Confirm Delete",
                              "Are you sure you want to delete '" +
                                  filtered_files_[index].name + "'?",
                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      QFile file(filtered_files_[index].filepath);
      if (file.remove()) {
        status_label_->setText("Deleted: " + filtered_files_[index].name);
        RefreshLoafFiles();
      } else {
        QMessageBox::warning(this, "Error", "Failed to delete file");
      }
    }
  }
}

void LoafBrowserWidget::UpdateLoafList() {
  file_list_->clear();
  filtered_files_.clear();

  QString search_text = search_edit_->text().toLower();

  for (const auto& loaf : loaf_files_) {
    if (!search_text.isEmpty()) {
      QString name = loaf.name.toLower();
      if (!name.contains(search_text)) {
        continue;
      }
    }

    filtered_files_.push_back(loaf);
  }

  for (const auto& loaf : filtered_files_) {
    QString display_text = loaf.name;
    if (loaf.itemCount > 0) {
      display_text += QString(" (%1 items)").arg(loaf.itemCount);
    }
    file_list_->addItem(display_text);
  }

  status_label_->setText(QString("Showing %1 of %2 loaf files")
                             .arg(filtered_files_.size())
                             .arg(loaf_files_.size()));
}

void LoafBrowserWidget::UpdateLoafPreview(const LoafFileInfo& info) {
  QString preview;

  preview += "<h3>" + info.name + "</h3>";

  if (!info.description.isEmpty()) {
    preview += "<p><b>Description:</b> " + info.description + "</p>";
  }

  preview += "<p><b>File:</b> <code>" + info.filepath + "</code></p>";
  preview += "<p><b>Items:</b> " + QString::number(info.itemCount) + "</p>";
  preview += "<p><b>Last Modified:</b> " + info.lastModified + "</p>";

  std::ifstream file(info.filepath.toStdString());
  if (file.is_open()) {
    preview += "<hr><h4>Content Preview:</h4>";
    preview += "<pre>";

    std::string line;
    int line_count = 0;
    while (std::getline(file, line) && line_count < 20) {
      preview += QString::fromStdString(line).toHtmlEscaped() + "\n";
      line_count++;
    }

    if (file.good()) {
      preview += "...\n(file continues)";
    }
    preview += "</pre>";
  }
  preview_text_->setHtml(preview);
}
}  // namespace BreadBin::GUI