#include <iostream>
#include <string>
#include "Loaf.h"
#include "LoafEditor.h"
#include "TextEditor.h"
#include "ThemeEditor.h"

using namespace BreadBin;

void PrintMenu ( ) {
    std::cout << "\n=== Bread Bin - Loaf Management System ===\n";
    std::cout << "1. Loaf Editor\n";
    std::cout << "2. Text Editor\n";
    std::cout << "3. Theme Editor\n";
    std::cout << "4. Exit\n";
    std::cout << "Select option: ";
}

void LoafEditorMenu ( ) {
    LoafEditor editor;
    std::string input;
    
    while (true) {
        std::cout << "\n--- Loaf Editor ---\n";
        std::cout << "1. Create new Loaf\n";
        std::cout << "2. Open Loaf\n";
        std::cout << "3. Add Application\n";
        std::cout << "4. Add File\n";
        std::cout << "5. Add Script\n";
        std::cout << "6. Save Loaf\n";
        std::cout << "7. Back to main menu\n";
        std::cout << "Select option: ";
        
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::cout << "Enter Loaf name: ";
            std::string name;
            std::getline(std::cin, name);
            editor.NewLoaf(name);
            std::cout << "Created new Loaf: " << name << "\n";
        }
        else {
            if (input == "2") {
                std::cout << "Enter Loaf filepath: ";
                std::string filepath;
                std::getline(std::cin, filepath);
                if (editor.OpenLoaf(filepath)) {
                    std::cout << "Opened Loaf successfully\n";
                }
                else {
                    std::cout << "Failed to open Loaf\n";
                }
            }
            else {
                if (input == "3") {
                    std::cout << "Enter application ID: ";
                    std::string id;
                    std::getline(std::cin, id);
                    std::cout << "Enter application name: ";
                    std::string name;
                    std::getline(std::cin, name);
                    std::cout << "Enter application path: ";
                    std::string path;
                    std::getline(std::cin, path);

                    if (editor.AddApplication(id, name, path)) {
                        std::cout << "Added application successfully\n";
                    }
                    else {
                        std::cout << "Failed to add application\n";
                    }
                }
                else {
                    if (input == "4") {
                        std::cout << "Enter file ID: ";
                        std::string id;
                        std::getline(std::cin, id);
                        std::cout << "Enter file name: ";
                        std::string name;
                        std::getline(std::cin, name);
                        std::cout << "Enter file path: ";
                        std::string path;
                        std::getline(std::cin, path);

                        if (editor.AddFile(id, name, path)) {
                            std::cout << "Added file successfully\n";
                        }
                        else {
                            std::cout << "Failed to add file\n";
                        }
                    }
                    else {
                        if (input == "5") {
                            std::cout << "Enter script ID: ";
                            std::string id;
                            std::getline(std::cin, id);
                            std::cout << "Enter script name: ";
                            std::string name;
                            std::getline(std::cin, name);
                            std::cout << "Enter script path: ";
                            std::string path;
                            std::getline(std::cin, path);

                            if (editor.AddScript(id, name, path)) {
                                std::cout << "Added script successfully\n";
                            }
                            else {
                                std::cout << "Failed to add script\n";
                            }
                        }
                        else {
                            if (input == "6") {
                                std::cout << "Enter save filepath: ";
                                std::string filepath;
                                std::getline(std::cin, filepath);
                                if (editor.SaveLoaf(filepath)) {
                                    std::cout << "Saved Loaf successfully\n";
                                }
                                else {
                                    std::cout << "Failed to save Loaf\n";
                                }
                            }
                            else {
                                if (input == "7") {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void TextEditorMenu() {
    TextEditor editor;
    std::string input;
    
    while (true) {
        std::cout << "\n--- Text Editor ---\n";
        std::cout << "1. Open file\n";
        std::cout << "2. Show content\n";
        std::cout << "3. Find text\n";
        std::cout << "4. Replace all\n";
        std::cout << "5. Save file\n";
        std::cout << "6. Back to main menu\n";
        std::cout << "Select option: ";
        
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::cout << "Enter file path: ";
            std::string filepath;
            std::getline(std::cin, filepath);
            if (editor.OpenFile(filepath)) {
                std::cout << "Opened file successfully\n";
            }
            else {
                std::cout << "Failed to open file\n";
            }
        }
        else {
            if (input == "2") {
                std::cout << "Content:\n" << editor.GetContent() << "\n";
            }
            else {
                if (input == "3") {
                    std::cout << "Enter search text: ";
                    std::string search_text;
                    std::getline(std::cin, search_text);
                    int line, column;
                    if (editor.Find(search_text, line, column)) {
                        std::cout << "Found at line " << line << ", column " << column << "\n";
                    }
                    else {
                        std::cout << "Not found\n";
                    }
                }
                else {
                    if (input == "4") {
                        std::cout << "Enter search text: ";
                        std::string search_text;
                        std::getline(std::cin, search_text);
                        std::cout << "Enter replace text: ";
                        std::string replace_text;
                        std::getline(std::cin, replace_text);
                        int count = editor.ReplaceAll(search_text, replace_text);
                        std::cout << "Replaced " << count << " occurrences\n";
                    }
                    else {
                        if (input == "5") {
                            std::cout << "Enter save filepath: ";
                            std::string filepath;
                            std::getline(std::cin, filepath);
                            if (editor.SaveFile(filepath)) {
                                std::cout << "Saved file successfully\n";
                            }
                            else {
                                std::cout << "Failed to save file\n";
                            }
                        }
                        else {
                            if (input == "6") {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ThemeEditorMenu() {
    ThemeEditor editor;
    std::string input;
    
    while (true) {
        std::cout << "\n--- Theme Editor ---\n";
        std::cout << "1. Create new theme\n";
        std::cout << "2. Load theme\n";
        std::cout << "3. Set colour\n";
        std::cout << "4. Set font\n";
        std::cout << "5. Save theme\n";
        std::cout << "6. Back to main menu\n";
        std::cout << "Select option: ";
        
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::cout << "Enter theme name: ";
            std::string name;
            std::getline(std::cin, name);
            editor.NewTheme(name);
            std::cout << "Created new theme: " << name << "\n";
        }
        else {
            if (input == "2") {
                std::cout << "Enter theme filepath: ";
                std::string filepath;
                std::getline(std::cin, filepath);
                if (editor.LoadTheme(filepath)) {
                    std::cout << "Loaded theme successfully\n";
                }
                else {
                    std::cout << "Failed to load theme\n";
                }
            }
            else {
                if (input == "3") {
                    std::cout << "Enter element name: ";
                    std::string element;
                    std::getline(std::cin, element);
                    std::cout << "Enter RED GREEN BLUE ALPHA values (0-255): ";
                    int red, green, blue, alpha;
                    std::cin >> red >> green >> blue >> alpha;
                    std::cin.ignore();
                    editor.SetColour(element, ThemeEditor::Colour(red, green, blue, alpha));
                    std::cout << "Set colour successfully\n";
                }
                else {
                    if (input == "4") {
                        std::cout << "Enter element name: ";
                        std::string element;
                        std::getline(std::cin, element);
                        std::cout << "Enter font name: ";
                        std::string font_name;
                        std::getline(std::cin, font_name);
                        std::cout << "Enter font size: ";
                        int font_size;
                        std::cin >> font_size;
                        std::cin.ignore();
                        editor.SetFont(element, font_name, font_size);
                        std::cout << "Set font successfully\n";
                    }
                    else {
                        if (input == "5") {
                            std::cout << "Enter save filepath: ";
                            std::string filepath;
                            std::getline(std::cin, filepath);
                            if (editor.SaveTheme(filepath)) {
                                std::cout << "Saved theme successfully\n";
                            }
                            else {
                                std::cout << "Failed to save theme\n";
                            }
                        }
                        else {
                            if (input == "6") {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

int main ( ) {
    std::cout << "Bread Bin - Cross-Platform Loaf Management System\n";
    std::cout << "Version 0.1\n";
    
    std::string input;
    while (true) {
        PrintMenu();
        std::getline(std::cin, input);
        
        if (input == "1") {
            LoafEditorMenu();
        }
        else {
            if (input == "2") {
                TextEditorMenu();
            }
            else {
                if (input == "3") {
                    ThemeEditorMenu();
                }
                else {
                    if (input == "4") {
                        std::cout << "Exiting Bread Bin. Goodbye!\n";
                        break;
                    }
                }
            }
        }
    }
    
    return 0;
}