#include "gui/TextEditorWidget.h"

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QShortcut>
#include <QStandardPaths>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

namespace BreadBin::GUI {
namespace {
class ScriptTerminalDialog final : public QDialog {
 public:
  explicit ScriptTerminalDialog(QWidget* parent = nullptr)
      : QDialog(parent), process_(new QProcess(this)) {
    setWindowTitle("Script Terminal");
    resize(900, 540);

    QVBoxLayout* layout = new QVBoxLayout(this);
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    output_->setStyleSheet(
        "background-color: #121212; color: #e6e6e6; font-family: monospace;");
    layout->addWidget(output_, 1);

    QHBoxLayout* input_layout = new QHBoxLayout();
    input_ = new QLineEdit(this);
    input_->setPlaceholderText("Type input for script and press Enter...");
    QPushButton* send_button = new QPushButton("Send", this);
    input_layout->addWidget(input_, 1);
    input_layout->addWidget(send_button);
    layout->addLayout(input_layout);

    connect(send_button, &QPushButton::clicked, this,
            &ScriptTerminalDialog::SendInput);
    connect(input_, &QLineEdit::returnPressed, this,
            &ScriptTerminalDialog::SendInput);
    connect(process_, &QProcess::readyReadStandardOutput, this, [this]() {
      AppendOutput(QString::fromUtf8(process_->readAllStandardOutput()));
    });
    connect(process_, &QProcess::readyReadStandardError, this, [this]() {
      AppendOutput(QString::fromUtf8(process_->readAllStandardError()));
    });
    connect(process_, &QProcess::finished, this,
            [this](int exit_code, QProcess::ExitStatus status) {
              const QString status_text =
                  status == QProcess::NormalExit ? "Normal" : "Crash";
              AppendOutput(QString("\n[process exited: code=%1, status=%2]\n")
                               .arg(exit_code)
                               .arg(status_text));
              input_->setEnabled(false);
            });
  }

  void Start(const QString& program, const QStringList& arguments) {
    output_->clear();
    input_->setEnabled(true);
    AppendOutput(QString("$ %1 %2\n").arg(program, arguments.join(' ')));
    process_->start(program, arguments);
    if (!process_->waitForStarted()) {
      AppendOutput(QString("Failed to start process: %1\n").arg(program));
      input_->setEnabled(false);
    }
  }

  void closeEvent(QCloseEvent* event) override {
    if (process_->state() != QProcess::NotRunning) {
      process_->terminate();
      process_->waitForFinished(1000);
    }
    QDialog::closeEvent(event);
  }

 private:
  void SendInput() {
    if (!input_->isEnabled() || process_->state() != QProcess::Running) {
      return;
    }

    const QString text = input_->text();
    process_->write((text + "\n").toUtf8());
    AppendOutput(QString("> %1\n").arg(text));
    input_->clear();
  }

  void AppendOutput(const QString& text) {
    output_->moveCursor(QTextCursor::End);
    output_->insertPlainText(text);
    output_->moveCursor(QTextCursor::End);
  }

  QTextEdit* output_;
  QLineEdit* input_;
  QProcess* process_;
};

class GenericSyntaxHighlighter final : public QSyntaxHighlighter {
 public:
  enum class Mode { Plain, Script, Theme, Loaf, Json, Yaml, Xml, Ini };

  explicit GenericSyntaxHighlighter(QTextDocument* parent, Mode mode)
      : QSyntaxHighlighter(parent), mode_(mode) {
    keyword_format_.setForeground(QColor("#7a4d13"));
    keyword_format_.setFontWeight(QFont::Bold);

    value_format_.setForeground(QColor("#1f5f8b"));
    comment_format_.setForeground(QColor("#6f6a61"));
    comment_format_.setFontItalic(true);
    string_format_.setForeground(QColor("#2b6f3f"));
  }

 protected:
  void highlightBlock(const QString& text) override {
    switch (mode_) {
      case Mode::Script:
        HighlightScript(text);
        break;
      case Mode::Theme:
        HighlightKeyValue(
            text, QRegularExpression(
                      "^(NAME|PRIMARY_COLOUR|SECONDARY_COLOUR|BACKGROUND_"
                      "COLOUR|TEXT_COLOUR|TEXT_COLOR|ACCENT_COLOUR|COLOUR_[A-"
                      "Za-z_]+|FONT_[A-Za-z_]+|USE_ELEMENT_FONTS)\\s*[:=]"));
        break;
      case Mode::Loaf:
        HighlightKeyValue(
            text,
            QRegularExpression(
                "^(TYPE|ID|NAME|PATH|META_COUNT|META_KEY|META_VALUE)\\s*:"));
        break;
      case Mode::Json:
        HighlightJson(text);
        break;
      case Mode::Yaml:
        HighlightKeyValue(text, QRegularExpression("^[A-Za-z0-9_\\-]+\\s*:"));
        break;
      case Mode::Xml:
        HighlightXml(text);
        break;
      case Mode::Ini:
        HighlightIni(text);
        break;
      case Mode::Plain:
      default:
        break;
    }
  }

 private:
  void HighlightScript(const QString& text) {
    static const QStringList keywords = {
        "if",   "then",     "else",    "fi",   "for",   "while",  "do",
        "done", "function", "return",  "def",  "class", "import", "from",
        "try",  "except",   "finally", "with", "async", "await"};
    for (const auto& keyword : keywords) {
      QRegularExpression keyword_regex(
          QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
      auto iterator = keyword_regex.globalMatch(text);
      while (iterator.hasNext()) {
        const auto match = iterator.next();
        setFormat(match.capturedStart(), match.capturedLength(),
                  keyword_format_);
      }
    }

    QRegularExpression hash_comment("#.*$");
    auto comment_match = hash_comment.match(text);
    if (comment_match.hasMatch()) {
      setFormat(comment_match.capturedStart(), comment_match.capturedLength(),
                comment_format_);
    }

    QRegularExpression quoted("\"[^\"]*\"|'[^']*'");
    auto string_iterator = quoted.globalMatch(text);
    while (string_iterator.hasNext()) {
      const auto match = string_iterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), string_format_);
    }
  }

  void HighlightKeyValue(const QString& text,
                         const QRegularExpression& key_regex) {
    const auto key_match = key_regex.match(text);
    if (key_match.hasMatch()) {
      setFormat(key_match.capturedStart(), key_match.capturedLength(),
                keyword_format_);
      const int separator_pos = text.indexOf(QRegularExpression("[:=]"));
      if (separator_pos >= 0 && separator_pos + 1 < text.size()) {
        setFormat(separator_pos + 1, text.size() - separator_pos - 1,
                  value_format_);
      }
    }

    if (text.trimmed().startsWith('#')) {
      setFormat(0, text.size(), comment_format_);
    }
  }

  void HighlightJson(const QString& text) {
    QRegularExpression key_regex("\"[^\"]+\"\\s*:");
    auto key_iterator = key_regex.globalMatch(text);
    while (key_iterator.hasNext()) {
      const auto match = key_iterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), keyword_format_);
    }

    QRegularExpression string_regex("\"[^\"]*\"");
    auto string_iterator = string_regex.globalMatch(text);
    while (string_iterator.hasNext()) {
      const auto match = string_iterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), string_format_);
    }
  }

  void HighlightXml(const QString& text) {
    QRegularExpression tag_regex("</?[A-Za-z0-9_:\\-]+[^>]*>");
    auto tag_iterator = tag_regex.globalMatch(text);
    while (tag_iterator.hasNext()) {
      const auto match = tag_iterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), keyword_format_);
    }
  }

  void HighlightIni(const QString& text) {
    if (text.trimmed().startsWith(';') || text.trimmed().startsWith('#')) {
      setFormat(0, text.size(), comment_format_);
      return;
    }

    QRegularExpression section_regex("^\\s*\\[[^\\]]+\\]");
    auto section_match = section_regex.match(text);
    if (section_match.hasMatch()) {
      setFormat(section_match.capturedStart(), section_match.capturedLength(),
                keyword_format_);
      return;
    }

    HighlightKeyValue(text, QRegularExpression("^[A-Za-z0-9_.\\-]+\\s*="));
  }

  Mode mode_;
  QTextCharFormat keyword_format_;
  QTextCharFormat value_format_;
  QTextCharFormat comment_format_;
  QTextCharFormat string_format_;
};

GenericSyntaxHighlighter::Mode ToHighlightMode(
    TextEditorWidget::DocumentType type) {
  switch (type) {
    case TextEditorWidget::DocumentType::Script:
      return GenericSyntaxHighlighter::Mode::Script;
    case TextEditorWidget::DocumentType::Theme:
      return GenericSyntaxHighlighter::Mode::Theme;
    case TextEditorWidget::DocumentType::Loaf:
      return GenericSyntaxHighlighter::Mode::Loaf;
    case TextEditorWidget::DocumentType::Json:
      return GenericSyntaxHighlighter::Mode::Json;
    case TextEditorWidget::DocumentType::Yaml:
      return GenericSyntaxHighlighter::Mode::Yaml;
    case TextEditorWidget::DocumentType::Xml:
      return GenericSyntaxHighlighter::Mode::Xml;
    case TextEditorWidget::DocumentType::Ini:
      return GenericSyntaxHighlighter::Mode::Ini;
    case TextEditorWidget::DocumentType::PlainText:
    default:
      return GenericSyntaxHighlighter::Mode::Plain;
  }
}
}  // namespace

TextEditorWidget::TextEditorWidget(QWidget* parent)
    : QWidget(parent), prompted_on_first_show_(false) {
  SetupUI();
  ConnectSignals();
}

TextEditorWidget::~TextEditorWidget() {}

void TextEditorWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);

  if (prompted_on_first_show_) {
    return;
  }

  prompted_on_first_show_ = true;

  const FirstOpenAction action = PromptForFirstOpenAction();
  if (action == FirstOpenAction::OpenExisting) {
    OnOpenFile();
    if (tab_widget_->count() == 0) {
      CreateNewTab("Untitled", DocumentType::PlainText);
      status_label_->setText("Ready");
    }
    return;
  }

  if (action == FirstOpenAction::Cancel) {
    CreateNewTab("Untitled", DocumentType::PlainText);
    status_label_->setText("Ready");
    return;
  }

  bool accepted = false;
  const DocumentType type = PromptForDocumentType(&accepted);
  if (!accepted) {
    CreateNewTab("Untitled", DocumentType::PlainText);
    status_label_->setText("Opened default text file");
    return;
  }

  if (type == DocumentType::Script) {
    NewScriptFile();
    return;
  }

  CreateNewTab("Untitled" + SuggestedExtensionForType(type), type);
  status_label_->setText("Created new " + SuggestedExtensionForType(type) +
                         " file");
}

void TextEditorWidget::SetupUI() {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(16, 16, 16, 16);
  main_layout->setSpacing(12);

  QHBoxLayout* toolbar_layout = new QHBoxLayout();
  toolbar_layout->setSpacing(8);

  new_file_button_ = new QPushButton("📄 New", this);
  open_file_button_ = new QPushButton("📂 Open", this);
  save_file_button_ = new QPushButton("💾 Save", this);
  save_file_button_->setToolTip("Save the current file");
  save_file_button_->setMinimumWidth(80);

  run_script_button_ = new QPushButton("▶️ Test Run", this);
  run_script_button_->setToolTip("Run the current script and show output");
  run_script_button_->setMinimumWidth(100);

  find_button_ = new QPushButton("🔍 Find", this);
  replace_button_ = new QPushButton("🔄 Replace", this);

  toolbar_layout->addWidget(new_file_button_);
  toolbar_layout->addWidget(open_file_button_);
  toolbar_layout->addWidget(save_file_button_);
  toolbar_layout->addWidget(run_script_button_);
  toolbar_layout->addSpacing(12);
  toolbar_layout->addWidget(find_button_);
  toolbar_layout->addWidget(replace_button_);
  toolbar_layout->addStretch();
  main_layout->addLayout(toolbar_layout);

  tab_widget_ = new QTabWidget(this);
  tab_widget_->setTabsClosable(true);
  tab_widget_->setMovable(true);
  tab_widget_->setDocumentMode(true);
  main_layout->addWidget(tab_widget_, 1);

  status_label_ = new QLabel("Ready", this);
  status_label_->setStyleSheet(
      "color: #8b7a5e; font-size: 12px; padding: 8px;");
  main_layout->addWidget(status_label_);

  new QShortcut(QKeySequence::Find, this, SLOT(OnFind()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this, SLOT(OnReplace()));
  new QShortcut(QKeySequence::Replace, this, SLOT(OnReplace()));

  setLayout(main_layout);
}

void TextEditorWidget::ConnectSignals() {
  connect(new_file_button_, &QPushButton::clicked, this,
          &TextEditorWidget::OnNewFile);
  connect(open_file_button_, &QPushButton::clicked, this,
          &TextEditorWidget::OnOpenFile);
  connect(save_file_button_, &QPushButton::clicked, this,
          &TextEditorWidget::OnSaveFile);
  connect(run_script_button_, &QPushButton::clicked, this,
          &TextEditorWidget::OnRunScript);
  connect(find_button_, &QPushButton::clicked, this, &TextEditorWidget::OnFind);
  connect(replace_button_, &QPushButton::clicked, this,
          &TextEditorWidget::OnReplace);
  connect(tab_widget_, &QTabWidget::currentChanged, this,
          &TextEditorWidget::OnTabChanged);
  connect(tab_widget_, &QTabWidget::tabCloseRequested, this,
          &TextEditorWidget::OnCloseTab);
}

void TextEditorWidget::CreateNewTab(const QString& title, DocumentType type) {
  QPlainTextEdit* editor = new QPlainTextEdit(this);

  QFont mono_font;
  mono_font.setPointSize(11);
  mono_font.setStyleHint(QFont::Monospace);
  mono_font.setFamilies({"Consolas", "Monaco", "Courier New", "monospace"});
  editor->setFont(mono_font);
  editor->setLineWrapMode(QPlainTextEdit::NoWrap);

  auto text_editor = std::make_shared<TextEditor>();
  editors_.push_back(text_editor);

  const int index = tab_widget_->addTab(editor, title);
  tab_widget_->setCurrentIndex(index);

  file_paths_.append("");
  document_types_.push_back(type);
  preferred_extensions_.append(SuggestedExtensionForType(type));
  script_loaf_names_.append("");

  ApplySyntaxHighlighting(index, type);
  connect(editor, &QPlainTextEdit::textChanged, this,
          &TextEditorWidget::OnTextChanged);
  UpdateRunScriptButtonState(index);
}

void TextEditorWidget::NewFile() { OnNewFile(); }

void TextEditorWidget::NewScriptFile(const QString& loaf_name) {
  bool accepted = false;
  const QString extension = PromptForScriptExtension(&accepted);
  if (!accepted) {
    return;
  }

  const QString script_title = "untitled_script" + extension;
  CreateNewTab(script_title, DocumentType::Script);

  const int index = GetCurrentTabIndex();
  if (index >= 0 && index < preferred_extensions_.size()) {
    preferred_extensions_[index] = extension;
  }
  if (index >= 0 && index < script_loaf_names_.size()) {
    script_loaf_names_[index] = loaf_name;
  }

  status_label_->setText("Created new script file");
}

void TextEditorWidget::OnNewFile() {
  bool accepted = false;
  const DocumentType type = PromptForDocumentType(&accepted);
  if (!accepted) {
    return;
  }

  if (type == DocumentType::Script) {
    NewScriptFile();
    return;
  }

  const QString extension = SuggestedExtensionForType(type);
  CreateNewTab("Untitled" + extension, type);
  const int index = GetCurrentTabIndex();
  if (index >= 0 && index < preferred_extensions_.size()) {
    preferred_extensions_[index] = extension;
  }
  status_label_->setText("Created new file");
}

void TextEditorWidget::OnOpenFile() {
  const QString filepath =
      QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*)");
  if (!filepath.isEmpty()) {
    OpenFile(filepath);
  }
}

bool TextEditorWidget::OpenFile(const QString& filepath) {
  if (tab_widget_->count() == 0) {
    CreateNewTab();
  }

  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error", "Could not open file: " + filepath);
    return false;
  }

  QTextStream in(&file);
  const QString content = in.readAll();
  file.close();

  const DocumentType type = DetectDocumentType(filepath, content);

  int current_index = GetCurrentTabIndex();
  QPlainTextEdit* editor = GetCurrentEditor();

  if (editor && editor->toPlainText().isEmpty() &&
      file_paths_[current_index].isEmpty()) {
    editor->setPlainText(content);
    file_paths_[current_index] = filepath;
    tab_widget_->setTabText(current_index, QFileInfo(filepath).fileName());
    preferred_extensions_[current_index] = "." + QFileInfo(filepath).suffix();

    if (current_index >= 0 &&
        current_index < static_cast<int>(document_types_.size())) {
      document_types_[current_index] = type;
      ApplySyntaxHighlighting(current_index, type);
    }
  } else {
    CreateNewTab(QFileInfo(filepath).fileName(), type);
    current_index = GetCurrentTabIndex();
    editor = GetCurrentEditor();
    editor->setPlainText(content);
    file_paths_[current_index] = filepath;
    preferred_extensions_[current_index] = "." + QFileInfo(filepath).suffix();
  }

  if (current_index >= 0 && current_index < static_cast<int>(editors_.size())) {
    editors_[current_index]->OpenFile(filepath.toStdString());
  }

  status_label_->setText("Opened: " + filepath);
  UpdateRunScriptButtonState(current_index);
  OnTabChanged(current_index);
  return true;
}

void TextEditorWidget::OnSaveFile() {
  if (tab_widget_->count() == 0) {
    CreateNewTab();
  }

  const int index = GetCurrentTabIndex();
  if (index < 0) {
    return;
  }

  QString filepath = file_paths_[index];
  if (filepath.isEmpty()) {
    QPlainTextEdit* editor = GetCurrentEditor();
    const QString content = editor ? editor->toPlainText() : "";
    const DocumentType detected_type = DetectDocumentType("", content);
    document_types_[index] = detected_type;

    QString default_dir = SuggestedDirectoryForType(detected_type);
    if (detected_type == DocumentType::Script &&
        index < script_loaf_names_.size() &&
        !script_loaf_names_[index].isEmpty()) {
      QString loaf_folder = script_loaf_names_[index];
      loaf_folder.replace(QRegularExpression("[^a-zA-Z0-9_\\-]"), "_");
      default_dir += "/" + loaf_folder;
    }
    QDir().mkpath(default_dir);

    const QString tab_name = tab_widget_->tabText(index).replace("*", "");
    const QString default_name =
        tab_name.isEmpty() ? QString("untitled") : tab_name;
    QString extension = index < preferred_extensions_.size()
                            ? preferred_extensions_[index]
                            : SuggestedExtensionForType(detected_type);
    if (extension.isEmpty()) {
      extension = SuggestedExtensionForType(detected_type);
    }
    const QString suggested_path = default_dir + "/" + default_name + extension;

    filepath =
        QFileDialog::getSaveFileName(this, "Save File", suggested_path,
                                     SuggestedFilterForType(detected_type));

    if (filepath.isEmpty()) return;

    if (!extension.isEmpty() && QFileInfo(filepath).suffix().isEmpty()) {
      filepath += extension;
    }

    file_paths_[index] = filepath;
    preferred_extensions_[index] = "." + QFileInfo(filepath).suffix();
    tab_widget_->setTabText(index, QFileInfo(filepath).fileName());
  }

  SaveFile(filepath);
}

bool TextEditorWidget::SaveFile(const QString& filepath) {
  auto* editor = GetCurrentEditor();
  if (!editor) {
    return false;
  }

  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error", "Could not save file: " + filepath);
    return false;
  }

  QTextStream out(&file);
  out << editor->toPlainText();
  file.close();

  const int current_index = GetCurrentTabIndex();
  if (current_index >= 0) {
    const QString cleaned_title = QFileInfo(filepath).fileName();
    tab_widget_->setTabText(current_index, cleaned_title);
    document_types_[current_index] =
        DetectDocumentType(filepath, editor->toPlainText());
    preferred_extensions_[current_index] = "." + QFileInfo(filepath).suffix();
    ApplySyntaxHighlighting(current_index, document_types_[current_index]);
    UpdateRunScriptButtonState(current_index);
  }

  status_label_->setText("Saved: " + filepath);
  return true;
}

bool TextEditorWidget::SaveCurrentFile() {
  if (tab_widget_->count() == 0) {
    CreateNewTab();
  }

  const int index = GetCurrentTabIndex();
  if (index < 0) {
    return false;
  }

  if (file_paths_[index].isEmpty()) {
    OnSaveFile();
    return true;
  }

  return SaveFile(file_paths_[index]);
}

bool TextEditorWidget::HasUnsavedChanges() const {
  for (int i = 0; i < tab_widget_->count(); ++i) {
    if (tab_widget_->tabText(i).endsWith('*')) {
      return true;
    }
  }
  return false;
}

void TextEditorWidget::OnRunScript() {
  const int current_index = GetCurrentTabIndex();
  if (current_index < 0) {
    return;
  }

  if (document_types_[current_index] != DocumentType::Script) {
    QMessageBox::information(this, "Test Run",
                             "Current file is not detected as a script.");
    return;
  }

  if (file_paths_[current_index].isEmpty()) {
    QMessageBox::information(this, "Test Run",
                             "Please save the script before running it.");
    return;
  }

  SaveFile(file_paths_[current_index]);

  const QString script_path = file_paths_[current_index];
  const QString suffix = QFileInfo(script_path).suffix().toLower();

  QString program;
  QStringList arguments;

#ifdef _WIN32
  if (suffix == "bat" || suffix == "cmd") {
    program = "cmd";
    arguments << "/c" << script_path;
  } else if (suffix == "ps1") {
    program = "powershell";
    arguments << "-ExecutionPolicy" << "Bypass" << "-File" << script_path;
  } else if (suffix == "py") {
    program = "python";
    arguments << script_path;
  } else if (suffix == "rb") {
    program = "ruby";
    arguments << script_path;
  } else if (suffix == "pl") {
    program = "perl";
    arguments << script_path;
  } else {
    program = "bash";
    arguments << script_path;
  }
#else
  if (suffix == "py") {
    program = "python3";
    arguments << script_path;
  } else if (suffix == "rb") {
    program = "ruby";
    arguments << script_path;
  } else if (suffix == "pl") {
    program = "perl";
    arguments << script_path;
  } else if (suffix == "ps1") {
    program = "pwsh";
    arguments << "-File" << script_path;
  } else {
    program = "bash";
    arguments << script_path;
  }
#endif

  ScriptTerminalDialog terminal(this);
  terminal.Start(program, arguments);
  terminal.exec();
  status_label_->setText("Test run finished for: " +
                         QFileInfo(script_path).fileName());
}

void TextEditorWidget::OnFind() {
  auto* editor = GetCurrentEditor();
  if (!editor) {
    return;
  }

  bool ok = false;
  const QString search_text = QInputDialog::getText(
      this, "Find", "Search for:", QLineEdit::Normal, "", &ok);
  if (ok && !search_text.isEmpty()) {
    if (editor->find(search_text)) {
      status_label_->setText("Found: " + search_text);
    } else {
      status_label_->setText("Not found: " + search_text);
    }
  }
}

void TextEditorWidget::OnReplace() {
  auto* editor = GetCurrentEditor();
  if (!editor) {
    return;
  }

  bool ok = false;
  const QString search_text = QInputDialog::getText(
      this, "Replace", "Search for:", QLineEdit::Normal, "", &ok);
  if (!ok || search_text.isEmpty()) {
    return;
  }

  const QString replace_text = QInputDialog::getText(
      this, "Replace", "Replace with:", QLineEdit::Normal, "", &ok);
  if (!ok) {
    return;
  }

  QString content = editor->toPlainText();
  const int count = content.count(search_text);
  if (count > 0) {
    content.replace(search_text, replace_text);
    editor->setPlainText(content);
    status_label_->setText(QString("Replaced %1 occurrence(s)").arg(count));
  } else {
    status_label_->setText("No occurrences found");
  }
}

void TextEditorWidget::OnTextChanged() {
  int currentIndex = GetCurrentTabIndex();
  if (currentIndex >= 0) {
    QString current_title = tab_widget_->tabText(currentIndex);
    if (!current_title.endsWith("*")) {
      tab_widget_->setTabText(currentIndex, current_title + "*");
    }

    emit FileModified();
  }
}

void TextEditorWidget::OnTabChanged(int index) {
  if (index < 0 || index >= file_paths_.size() ||
      index >= static_cast<int>(document_types_.size())) {
    status_label_->setText("Ready");
    run_script_button_->setEnabled(false);
    return;
  }

  status_label_->setText(file_paths_[index].isEmpty()
                             ? "Editing: Untitled"
                             : "Editing: " + file_paths_[index]);

  UpdateRunScriptButtonState(index);
}

void TextEditorWidget::OnCloseTab(int index) {
  if (index < 0 || index >= tab_widget_->count()) {
    return;
  }

  tab_widget_->removeTab(index);
  if (index < static_cast<int>(editors_.size())) {
    editors_.erase(editors_.begin() + index);
  }
  if (index < file_paths_.size()) {
    file_paths_.removeAt(index);
  }
  if (index < static_cast<int>(document_types_.size())) {
    document_types_.erase(document_types_.begin() + index);
  }
  if (index < preferred_extensions_.size()) {
    preferred_extensions_.removeAt(index);
  }
  if (index < script_loaf_names_.size()) {
    script_loaf_names_.removeAt(index);
  }

  if (tab_widget_->count() == 0) {
    CreateNewTab();
  }
}

int TextEditorWidget::GetCurrentTabIndex() const {
  return tab_widget_->currentIndex();
}

QPlainTextEdit* TextEditorWidget::GetCurrentEditor() const {
  const int index = GetCurrentTabIndex();
  if (index >= 0) {
    return qobject_cast<QPlainTextEdit*>(tab_widget_->widget(index));
  }
  return nullptr;
}

TextEditorWidget::DocumentType TextEditorWidget::DetectDocumentType(
    const QString& filepath, const QString& content) const {
  const QString suffix = QFileInfo(filepath).suffix().toLower();

  if (suffix == "theme") return DocumentType::Theme;
  if (suffix == "loaf") return DocumentType::Loaf;
  if (suffix == "sh" || suffix == "py" || suffix == "rb" || suffix == "pl" ||
      suffix == "ps1" || suffix == "bat" || suffix == "cmd")
    return DocumentType::Script;
  if (suffix == "json") return DocumentType::Json;
  if (suffix == "yaml" || suffix == "yml") return DocumentType::Yaml;
  if (suffix == "xml") return DocumentType::Xml;
  if (suffix == "ini" || suffix == "cfg" || suffix == "conf")
    return DocumentType::Ini;

  if (content.contains("NAME=") && content.contains("PRIMARY_COLOUR=")) {
    return DocumentType::Theme;
  }

  if (content.contains("TYPE:") && content.contains("ID:") &&
      content.contains("META_COUNT:")) {
    return DocumentType::Loaf;
  }

  if (content.startsWith("#!/") || content.contains("import ") ||
      content.contains("def ") || content.contains("echo ")) {
    return DocumentType::Script;
  }

  return DocumentType::PlainText;
}

QString TextEditorWidget::SuggestedDirectoryForType(DocumentType type) const {
  const QString docs =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
      "/BreadBin";

  switch (type) {
    case DocumentType::Script:
      return docs + "/scripts";
    case DocumentType::Theme:
      return docs + "/themes";
    case DocumentType::Loaf:
      return docs + "/loafs";
    default:
      return docs;
  }
}

QString TextEditorWidget::SuggestedExtensionForType(DocumentType type) const {
  switch (type) {
    case DocumentType::Script:
      return ".sh";
    case DocumentType::Theme:
      return ".theme";
    case DocumentType::Loaf:
      return ".loaf";
    case DocumentType::Json:
      return ".json";
    case DocumentType::Yaml:
      return ".yaml";
    case DocumentType::Xml:
      return ".xml";
    case DocumentType::Ini:
      return ".ini";
    case DocumentType::PlainText:
    default:
      return ".txt";
  }
}

TextEditorWidget::DocumentType TextEditorWidget::PromptForDocumentType(
    bool* accepted, bool scripts_only) const {
  QStringList options;
  if (scripts_only) {
    options << "Script";
  } else {
    options << "Plain text (.txt)" << "Script" << "Theme (.theme)"
            << "Loaf (.loaf)"
            << "JSON (.json)" << "YAML (.yaml)" << "XML (.xml)" << "INI (.ini)";
  }

  bool ok = false;
  const QString choice = QInputDialog::getItem(
      const_cast<TextEditorWidget*>(this), "New File Type",
      "What kind of file do you want to create?", options, 0, false, &ok);

  if (accepted) {
    *accepted = ok;
  }

  if (!ok) {
    return DocumentType::PlainText;
  }

  if (choice.startsWith("Script")) return DocumentType::Script;
  if (choice.startsWith("Theme")) return DocumentType::Theme;
  if (choice.startsWith("Loaf")) return DocumentType::Loaf;
  if (choice.startsWith("JSON")) return DocumentType::Json;
  if (choice.startsWith("YAML")) return DocumentType::Yaml;
  if (choice.startsWith("XML")) return DocumentType::Xml;
  if (choice.startsWith("INI")) return DocumentType::Ini;

  return DocumentType::PlainText;
}

QString TextEditorWidget::PromptForScriptExtension(bool* accepted) const {
  QStringList options = {"Shell (.sh)",   "Python (.py)",      "Ruby (.rb)",
                         "Perl (.pl)",    "PowerShell (.ps1)", "Batch (.bat)",
                         "Command (.cmd)"};
  bool ok = false;
  const QString choice = QInputDialog::getItem(
      const_cast<TextEditorWidget*>(this), "Script Type",
      "What kind of script do you want to write?", options, 0, false, &ok);

  if (accepted) {
    *accepted = ok;
  }

  if (!ok) {
    return ".sh";
  }

  if (choice.contains(".py")) return ".py";
  if (choice.contains(".rb")) return ".rb";
  if (choice.contains(".pl")) return ".pl";
  if (choice.contains(".ps1")) return ".ps1";
  if (choice.contains(".bat")) return ".bat";
  if (choice.contains(".cmd")) return ".cmd";
  return ".sh";
}

TextEditorWidget::FirstOpenAction TextEditorWidget::PromptForFirstOpenAction()
    const {
  QDialog dialog(const_cast<TextEditorWidget*>(this));
  dialog.setWindowTitle("Welcome to the Text Editor");

  auto* layout = new QVBoxLayout(&dialog);
  auto* prompt = new QLabel("How would you like to get started?", &dialog);
  layout->addWidget(prompt);

  auto* actions = new QDialogButtonBox(&dialog);
  QPushButton* open_button =
      actions->addButton("Open Existing File", QDialogButtonBox::ActionRole);
  QPushButton* new_button =
      actions->addButton("Create New File", QDialogButtonBox::ActionRole);
  QPushButton* cancel_button = actions->addButton(QDialogButtonBox::Cancel);
  layout->addWidget(actions);

  FirstOpenAction selected_action = FirstOpenAction::Cancel;
  connect(open_button, &QPushButton::clicked, &dialog, [&]() {
    selected_action = FirstOpenAction::OpenExisting;
    dialog.accept();
  });
  connect(new_button, &QPushButton::clicked, &dialog, [&]() {
    selected_action = FirstOpenAction::CreateNew;
    dialog.accept();
  });
  connect(cancel_button, &QPushButton::clicked, &dialog, [&]() {
    selected_action = FirstOpenAction::Cancel;
    dialog.reject();
  });

  dialog.exec();
  return selected_action;
}

QString TextEditorWidget::SuggestedFilterForType(DocumentType type) const {
  switch (type) {
    case DocumentType::Script:
      return "Scripts (*.sh *.py *.rb *.pl *.ps1 *.bat *.cmd);;All Files (*)";
    case DocumentType::Theme:
      return "Theme Files (*.theme);;All Files (*)";
    case DocumentType::Loaf:
      return "Loaf Files (*.loaf);;All Files (*)";
    case DocumentType::Json:
      return "JSON Files (*.json);;All Files (*)";
    case DocumentType::Yaml:
      return "YAML Files (*.yaml *.yml);;All Files (*)";
    case DocumentType::Xml:
      return "XML Files (*.xml);;All Files (*)";
    case DocumentType::Ini:
      return "INI Files (*.ini *.cfg *.conf);;All Files (*)";
    case DocumentType::PlainText:
    default:
      return "Text Files (*.txt);;All Files (*)";
  }
}

void TextEditorWidget::ApplySyntaxHighlighting(int index, DocumentType type) {
  if (index < 0 || index >= tab_widget_->count()) {
    return;
  }

  auto* editor = qobject_cast<QPlainTextEdit*>(tab_widget_->widget(index));
  if (!editor) {
    return;
  }

  new GenericSyntaxHighlighter(editor->document(), ToHighlightMode(type));
}

void TextEditorWidget::UpdateRunScriptButtonState(int index) {
  const bool index_valid =
      index >= 0 && index < static_cast<int>(document_types_.size());
  run_script_button_->setEnabled(index_valid && document_types_[index] ==
                                                    DocumentType::Script);
}
}  // namespace BreadBin::GUI