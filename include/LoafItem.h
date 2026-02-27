#ifndef LOAF_ITEM_H
#define LOAF_ITEM_H

#include <map>
#include <string>

namespace BreadBin {
class LoafItem {
 public:
  enum class Type { APPLICATION, FILE, CONFIG, SCRIPT, WEBPAGE };

  LoafItem(std::string id, Type type);
  virtual ~LoafItem();

  [[nodiscard]] std::string GetId() const;
  [[nodiscard]] Type GetType() const;
  void SetName(const std::string& name);
  [[nodiscard]] std::string GetName() const;
  void SetPath(const std::string& path);
  [[nodiscard]] std::string GetPath() const;
  void SetMetadata(const std::string& key, const std::string& value);
  [[nodiscard]] std::string GetMetadata(const std::string& key) const;
  virtual bool Execute() = 0;
  virtual bool Validate() const = 0;
  [[nodiscard]] virtual std::string ToString() const;

 protected:
  std::string id_;
  Type type_;
  std::string name_;
  std::string path_;
  std::map<std::string, std::string> metadata_;
};

class ApplicationItem : public LoafItem {
 public:
  explicit ApplicationItem(const std::string& id);
  bool Execute() override;
  [[nodiscard]] bool Validate() const override;
};

class FileItem : public LoafItem {
 public:
  explicit FileItem(const std::string& id);
  bool Execute() override;
  [[nodiscard]] bool Validate() const override;
};

class ConfigItem : public LoafItem {
 public:
  explicit ConfigItem(const std::string& id);
  bool Execute() override;
  [[nodiscard]] bool Validate() const override;
};

class ScriptItem : public LoafItem {
 public:
  explicit ScriptItem(const std::string& id);
  bool Execute() override;
  [[nodiscard]] bool Validate() const override;
};

class WebPageItem : public LoafItem {
 public:
  explicit WebPageItem(const std::string& id);
  bool Execute() override;
  [[nodiscard]] bool Validate() const override;
};
}  // namespace BreadBin

#endif  // LOAF_ITEM_H