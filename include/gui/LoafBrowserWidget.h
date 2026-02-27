#ifndef LOAFBROWSERWIDGET_H
#define LOAFBROWSERWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QWidget>
#include <vector>

namespace BreadBin::GUI {
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
  explicit LoafBrowserWidget(QWidget* parent = nullptr);
  ~LoafBrowserWidget() override;

  void RefreshLoafFiles();
  void SetSearchPaths(const QStringList& paths);

 signals:
  void LoafSelected(const QString& filepath);
  void LoafOpened(const QString& filepath);

 private slots:
  void OnRefreshClicked();
  void OnSearchChanged(const QString& text);
  void OnLoafSelected();
  void OnOpenClicked();
  void OnDeleteClicked();

 private:
  void SetupUI();
  void ConnectSignals();
  void UpdateLoafList();
  void UpdateLoafPreview(const LoafFileInfo& info);
  void ScanForLoafFiles();
  LoafFileInfo LoadLoafInfo(const QString& filepath);

  std::vector<LoafFileInfo> loaf_files_;
  std::vector<LoafFileInfo> filtered_files_;
  QStringList search_paths_;
  QLineEdit* search_edit_;
  QListWidget* file_list_;
  QTextEdit* preview_text_;
  QPushButton* refresh_button_;
  QPushButton* open_button_;
  QPushButton* delete_button_;
  QLabel* status_label_;
  QLabel* paths_label_;
};
}  // namespace BreadBin::GUI

#endif  // LOAFBROWSERWIDGET_H