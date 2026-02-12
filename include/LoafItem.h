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

            LoafItem (const std::string& id, Type type);
            virtual ~LoafItem ( );

            std::string GetId ( ) const;
            Type GetType ( ) const;
            void SetName (const std::string& name);
            std::string GetName ( ) const;
            void SetPath (const std::string& path);
            std::string GetPath ( ) const;

            void SetMetadata (const std::string& key, const std::string& value);
            std::string GetMetadata (const std::string& key) const;

            virtual bool Execute ( ) = 0;
            virtual bool Validate ( ) const = 0;
            virtual std::string ToString ( ) const;

        protected:
            std::string id_;
            Type type_;
            std::string name_;
            std::string path_;
            std::map<std::string, std::string> metadata_;
    };

    class ApplicationItem : public LoafItem {
        public:
            explicit ApplicationItem (const std::string& id);
            bool Execute ( ) override;
            bool Validate ( ) const override;
    };

    class FileItem : public LoafItem {
        public:
            explicit FileItem (const std::string& id);
            bool Execute ( ) override;
            bool Validate ( ) const override;
    };

    class ConfigItem : public LoafItem {
        public:
            explicit ConfigItem (const std::string& id);
            bool Execute ( ) override;
            bool Validate ( ) const override;
    };

    class ScriptItem : public LoafItem {
        public:
            explicit ScriptItem (const std::string& id);
            bool Execute ( ) override;
            bool Validate ( ) const override;
    };

    class WebPageItem : public LoafItem {
        public:
            explicit WebPageItem (const std::string& id);
            bool Execute  ( ) override;
            bool Validate ( ) const override;
    };

} // namespace BreadBin

#endif // LOAF_ITEM_H
