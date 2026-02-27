#include "gui/HomeWidget.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextStream>
#include <QVBoxLayout>

namespace BreadBin::GUI {
HomeWidget::HomeWidget(QWidget* parent)
    : QWidget(parent),
      recent_list_(nullptr),
      favorites_list_(nullptr),
      categories_list_(nullptr),
      status_label_(nullptr) {
  SetupUI();
  ConnectSignals();
  LoadSettings();
  Refresh();
}

HomeWidget::~HomeWidget() { SaveSettings(); }

void HomeWidget::SetupUI() {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(20, 20, 20, 20);
  main_layout->setSpacing(20);

  QLabel* welcome_label = new QLabel(
      "<h1 style='color: #5d4e37; font-weight: 600;'>🍞 Welcome to Bread "
      "Bin</h1>",
      this);
  welcome_label->setAlignment(Qt::AlignCenter);
  welcome_label->setStyleSheet("padding: 20px; background-color: transparent;");
  main_layout->addWidget(welcome_label);

  QLabel* subtitle_label = new QLabel(
      "Organize and manage your applications, files, and workflows", this);
  subtitle_label->setAlignment(Qt::AlignCenter);
  subtitle_label->setStyleSheet(
      "color: #8b7a5e; font-size: 14px; padding-bottom: 10px;");
  main_layout->addWidget(subtitle_label);

  QHBoxLayout* action_layout = new QHBoxLayout();
  action_layout->setSpacing(12);
  action_layout->addStretch();

  QPushButton* new_loaf_btn = new QPushButton("➕ Create New Loaf", this);
  new_loaf_btn->setMinimumHeight(50);
  new_loaf_btn->setMinimumWidth(200);
  new_loaf_btn->setStyleSheet("font-size: 14px; font-weight: 600;");
  connect(new_loaf_btn, &QPushButton::clicked, this,
          &HomeWidget::NewLoafRequested);
  action_layout->addWidget(new_loaf_btn);

  action_layout->addStretch();
  main_layout->addLayout(action_layout);

  main_layout->addSpacing(10);

  QHBoxLayout* content_layout = new QHBoxLayout();
  content_layout->setSpacing(16);

  QGroupBox* recent_group = new QGroupBox("📂 Recently Opened", this);
  recent_group->setMinimumWidth(250);
  QVBoxLayout* recent_layout = new QVBoxLayout(recent_group);
  recent_layout->setContentsMargins(12, 20, 12, 12);
  recent_list_ = new QListWidget(this);
  recent_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  recent_list_->setMinimumHeight(200);
  recent_layout->addWidget(recent_list_);
  content_layout->addWidget(recent_group,
                            1);  // Stretch factor for responsive sizing

  QGroupBox* favorites_group = new QGroupBox("⭐ Favorites", this);
  favorites_group->setMinimumWidth(250);
  QVBoxLayout* favorites_layout = new QVBoxLayout(favorites_group);
  favorites_layout->setContentsMargins(12, 20, 12, 12);
  favorites_list_ = new QListWidget(this);
  favorites_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  favorites_list_->setMinimumHeight(200);
  favorites_layout->addWidget(favorites_list_);
  content_layout->addWidget(favorites_group,
                            1);  // Stretch factor for responsive sizing

  QGroupBox* categories_group = new QGroupBox("📁 Categories", this);
  categories_group->setMinimumWidth(250);
  QVBoxLayout* categories_layout = new QVBoxLayout(categories_group);
  categories_layout->setContentsMargins(12, 20, 12, 12);
  categories_list_ = new QListWidget(this);
  categories_list_->setContextMenuPolicy(Qt::CustomContextMenu);
  categories_list_->setMinimumHeight(200);
  categories_layout->addWidget(categories_list_);
  content_layout->addWidget(categories_group,
                            1);  // Stretch factor for responsive sizing

  main_layout->addLayout(content_layout, 1);  // Takes most space

  status_label_ = new QLabel("Ready", this);
  status_label_->setStyleSheet(
      "color: #8b7a5e; font-size: 12px; padding: 8px;");
  status_label_->setAlignment(Qt::AlignCenter);
  main_layout->addWidget(status_label_);

  setLayout(main_layout);
}

void HomeWidget::ConnectSignals() {
  connect(recent_list_, &QListWidget::itemDoubleClicked, this,
          &HomeWidget::OnRecentLoafDoubleClicked);
  connect(favorites_list_, &QListWidget::itemDoubleClicked, this,
          &HomeWidget::OnFavoriteLoafDoubleClicked);
  connect(categories_list_, &QListWidget::itemDoubleClicked, this,
          &HomeWidget::OnCategoryLoafDoubleClicked);

  connect(recent_list_, &QListWidget::customContextMenuRequested, this,
          &HomeWidget::ShowContextMenu);
  connect(favorites_list_, &QListWidget::customContextMenuRequested, this,
          &HomeWidget::ShowContextMenu);
  connect(categories_list_, &QListWidget::customContextMenuRequested, this,
          &HomeWidget::ShowContextMenu);
}

QString HomeWidget::GetSettingsFile() const {
  const QString config_dir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir().mkpath(config_dir);
  return config_dir + "/loaf_home.json";
}

void HomeWidget::LoadSettings() {
  QFile file(GetSettingsFile());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (!doc.isObject()) {
    return;
  }

  QJsonObject root = doc.object();

  if (root.contains("recent") && root["recent"].isArray()) {
    QJsonArray recent = root["recent"].toArray();
    recent_loafs_.clear();

    for (const QJsonValue& value : recent) {
      if (!value.isObject()) continue;
      QJsonObject obj = value.toObject();

      LoafInfo info;
      info.filepath = obj["filepath"].toString();
      info.name = obj["name"].toString();
      info.last_opened =
          QDateTime::fromString(obj["lastOpened"].toString(), Qt::ISODate);
      info.is_favorite = obj["isFavorite"].toBool();
      info.category = obj["category"].toString();

      if (!info.filepath.isEmpty() && QFile::exists(info.filepath)) {
        recent_loafs_.append(info);
        all_loafs_[info.filepath] = info;
      }
    }
  }
}

void HomeWidget::SaveSettings() {
  QJsonObject root;
  QJsonArray recent;

  for (const LoafInfo& info : recent_loafs_) {
    QJsonObject obj;
    obj["filepath"] = info.filepath;
    obj["name"] = info.name;
    obj["lastOpened"] = info.last_opened.toString(Qt::ISODate);
    obj["isFavorite"] = info.is_favorite;
    obj["category"] = info.category;
    recent.append(obj);
  }

  root["recent"] = recent;

  QFile file(GetSettingsFile());
  if (file.open(QIODevice::WriteOnly)) {
    QJsonDocument doc(root);
    file.write(doc.toJson());
  }
}

void HomeWidget::AddRecentLoaf(const QString& filepath, const QString& name) {
  if (filepath.isEmpty()) return;

  LoafInfo info;
  info.filepath = filepath;
  info.name = name.isEmpty() ? QFileInfo(filepath).completeBaseName() : name;
  info.last_opened = QDateTime::currentDateTime();

  if (all_loafs_.contains(filepath)) {
    info.is_favorite = all_loafs_[filepath].is_favorite;
    info.category = all_loafs_[filepath].category;

    for (int i = 0; i < recent_loafs_.size(); ++i) {
      if (recent_loafs_[i].filepath == filepath) {
        recent_loafs_.removeAt(i);
        break;
      }
    }
  }

  recent_loafs_.prepend(info);

  while (recent_loafs_.size() > 10) {
    recent_loafs_.removeLast();
  }

  all_loafs_[filepath] = info;
  SaveSettings();
  Refresh();
}

void HomeWidget::Refresh() {
  UpdateRecentList();
  UpdateFavoritesList();
  UpdateCategoriesList();
}

void HomeWidget::UpdateRecentList() {
  recent_list_->clear();

  for (const LoafInfo& info : recent_loafs_) {
    QString display = info.name;
    if (info.is_favorite) {
      display = "⭐ " + display;
    }
    if (!info.category.isEmpty()) {
      display += " [" + info.category + "]";
    }

    QListWidgetItem* item = new QListWidgetItem(display, recent_list_);
    item->setData(Qt::UserRole, info.filepath);
    item->setToolTip(info.filepath);
  }

  if (recent_loafs_.isEmpty()) {
    QListWidgetItem* item =
        new QListWidgetItem("No recent loafs", recent_list_);
    item->setFlags(Qt::NoItemFlags);
  }
}

void HomeWidget::UpdateFavoritesList() {
  favorites_list_->clear();

  for (const LoafInfo& info : all_loafs_) {
    if (info.is_favorite) {
      QString display = info.name;
      if (!info.category.isEmpty()) {
        display += " [" + info.category + "]";
      }

      QListWidgetItem* item = new QListWidgetItem(display, favorites_list_);
      item->setData(Qt::UserRole, info.filepath);
      item->setToolTip(info.filepath);
    }
  }

  if (favorites_list_->count() == 0) {
    QListWidgetItem* item =
        new QListWidgetItem("No favorites yet", favorites_list_);
    item->setFlags(Qt::NoItemFlags);
  }
}

void HomeWidget::UpdateCategoriesList() {
  categories_list_->clear();

  QMap<QString, QList<LoafInfo>> categories;

  for (const LoafInfo& info : all_loafs_) {
    QString cat = info.category.isEmpty() ? "Uncategorized" : info.category;
    categories[cat].append(info);
  }

  for (auto it = categories.begin(); it != categories.end(); ++it) {
    QListWidgetItem* header =
        new QListWidgetItem("📁 " + it.key(), categories_list_);
    QFont font = header->font();
    font.setBold(true);
    header->setFont(font);
    header->setFlags(Qt::NoItemFlags);
    header->setBackground(QColor(230, 230, 230));

    for (const LoafInfo& info : it.value()) {
      QString display = "  • " + info.name;
      if (info.is_favorite) {
        display = "  ⭐ " + info.name;
      }
      QListWidgetItem* item = new QListWidgetItem(display, categories_list_);
      item->setData(Qt::UserRole, info.filepath);
      item->setToolTip(info.filepath);
    }
  }

  if (categories_list_->count() == 0) {
    QListWidgetItem* item =
        new QListWidgetItem("No categorized loafs", categories_list_);
    item->setFlags(Qt::NoItemFlags);
  }
}

void HomeWidget::OnRecentLoafDoubleClicked(QListWidgetItem* item) {
  if (!item) return;
  QString filepath = item->data(Qt::UserRole).toString();
  if (!filepath.isEmpty()) {
    emit OpenLoafRequested(filepath);
  }
}

void HomeWidget::OnFavoriteLoafDoubleClicked(QListWidgetItem* item) {
  if (!item) return;
  QString filepath = item->data(Qt::UserRole).toString();
  if (!filepath.isEmpty()) {
    emit OpenLoafRequested(filepath);
  }
}

void HomeWidget::OnCategoryLoafDoubleClicked(QListWidgetItem* item) {
  if (!item) return;
  QString filepath = item->data(Qt::UserRole).toString();
  if (!filepath.isEmpty()) {
    emit OpenLoafRequested(filepath);
  }
}

void HomeWidget::ToggleFavorite(const QString& filepath) {
  if (all_loafs_.contains(filepath)) {
    all_loafs_[filepath].is_favorite = !all_loafs_[filepath].is_favorite;

    for (LoafInfo& info : recent_loafs_) {
      if (info.filepath == filepath) {
        info.is_favorite = all_loafs_[filepath].is_favorite;
        break;
      }
    }
    SaveSettings();
    Refresh();
  }
}

void HomeWidget::SetCategory(const QString& filepath, const QString& category) {
  if (all_loafs_.contains(filepath)) {
    all_loafs_[filepath].category = category;

    for (LoafInfo& info : recent_loafs_) {
      if (info.filepath == filepath) {
        info.category = category;
        break;
      }
    }

    SaveSettings();
    Refresh();
  }
}

void HomeWidget::ShowContextMenu(const QPoint& pos) {
  QListWidget* list_widget = qobject_cast<QListWidget*>(sender());
  if (!list_widget) return;

  QListWidgetItem* item = list_widget->itemAt(pos);
  if (!item) return;

  QString filepath = item->data(Qt::UserRole).toString();
  if (filepath.isEmpty()) return;

  QMenu menu(this);

  QAction* open_action = menu.addAction("Open in Loaf Editor");
  QAction* open_text_action = menu.addAction("Open in Text Editor");
  menu.addSeparator();

  bool is_favorite =
      all_loafs_.contains(filepath) && all_loafs_[filepath].is_favorite;
  QAction* favorite_action = menu.addAction(
      is_favorite ? "Remove from Favorites" : "Add to Favorites");

  QAction* category_action = menu.addAction("Set Category...");

  QAction* selected = menu.exec(list_widget->mapToGlobal(pos));

  if (selected == open_action) {
    emit OpenLoafRequested(filepath);
  } else {
    if (selected == open_text_action) {
      emit OpenLoafInTextEditorRequested(filepath);
    } else {
      if (selected == favorite_action) {
        ToggleFavorite(filepath);
      } else {
        if (selected == category_action) {
          bool ok;
          QString current_category = all_loafs_.contains(filepath)
                                         ? all_loafs_[filepath].category
                                         : "";
          QString category = QInputDialog::getText(
              this, "Set Category", "Category name:", QLineEdit::Normal,
              current_category, &ok);
          if (ok) {
            SetCategory(filepath, category);
          }
        }
      }
    }
  }
}
}  // namespace BreadBin::GUI