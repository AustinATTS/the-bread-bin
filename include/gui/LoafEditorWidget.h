#ifndef LOAFEDITORWIDGET_H
#define LOAFEDITORWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <memory>

#include "LoafEditor.h"

namespace BreadBin::GUI {
class LoafEditorWidget : public QWidget {
  Q_OBJECT

 public:
  explicit LoafEditorWidget(QWidget* parent = nullptr);
  ~LoafEditorWidget() override;

  void NewLoaf();
  bool OpenLoaf(const QString& filepath);
  bool SaveLoaf(const QString& filepath);
  bool SaveLoaf();
  [[nodiscard]] bool HasUnsavedChanges() const;
  [[nodiscard]] std::shared_ptr<Loaf> GetCurrentLoaf() const;
  [[nodiscard]] QString GetCurrentFilePath() const;

 signals:
  void LoafModified();
  void CreateNewScriptRequested(const QString& loaf_name);

 private slots:
  void OnAddApplication();
  void OnAddFile();
  void OnAddScript();
  void OnAddConfig();
  void OnAddWebPage();
  void OnRemoveItem();
  void OnItemSelected();
  void OnConfigureItem();
  void OnNameChanged();
  void OnDescriptionChanged();

 private:
  void SetupUI();
  void UpdateLoafInfo();
  void RefreshItemList();
  void ConnectSignals();
  [[nodiscard]] QString GenerateItemId(const QString& name) const;

  std::shared_ptr<LoafEditor> editor_;
  QString current_file_path_;
  QLineEdit* name_edit_;
  QTextEdit* description_edit_;
  QListWidget* item_list_;
  QPushButton* add_app_button_;
  QPushButton* add_file_button_;
  QPushButton* add_script_button_;
  QPushButton* add_config_button_;
  QPushButton* add_web_page_button_;
  QPushButton* remove_button_;
  QPushButton* configure_button_;
  QLabel* status_label_;
};
}  // namespace BreadBin::GUI

#endif  // LOAFEDITORWIDGET_H