#ifndef LOAF_ITEM_H
#define LOAF_ITEM_H

#include <string>
#include <map>

namespace BreadBin {

    class LoafItem {
        public:
            enum class Type {
                APPLICATION,
                FILE,
                CONFIG,
                SCRIPT,
                WEBPAGE
            };

            LoafItem(const std::string& id, Type type);
            virtual ~LoafItem();

            std::string getId() const;
            Type getType() const;
            void setName(const std::string& name);
            std::string getName() const;
            void setPath(const std::string& path);
            std::string getPath() const;

            void setMetadata(const std::string& key, const std::string& value);
            std::string getMetadata(const std::string& key) const;

            virtual bool execute() = 0;
            virtual bool validate() const = 0;
            virtual std::string toString() const;

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
            bool execute() override;
            bool validate() const override;
    };

    class FileItem : public LoafItem {
        public:
            explicit FileItem(const std::string& id);
            bool execute() override;
            bool validate() const override;
    };

    class ConfigItem : public LoafItem {
        public:
            explicit ConfigItem(const std::string& id);
            bool execute() override;
            bool validate() const override;
    };

    class ScriptItem : public LoafItem {
        public:
            explicit ScriptItem(const std::string& id);
            bool execute() override;
            bool validate() const override;
    };

    class WebPageItem : public LoafItem {
        public:
            explicit WebPageItem(const std::string& id);
            bool execute() override;
            bool validate() const override;
    };

} // namespace BreadBin

#endif // LOAF_ITEM_H
