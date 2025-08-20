#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

class DebugConfig {
private:
    static DebugConfig* instance;
    bool m_renderDebug;
    bool m_shadowDebug;
    
    DebugConfig() : m_renderDebug(false), m_shadowDebug(false) {
        loadConfig();
    }
    
public:
    static DebugConfig& getInstance() {
        if (instance == nullptr) {
            instance = new DebugConfig();
        }
        return *instance;
    }
    
    void loadConfig() {
        try {
            std::ifstream f("config.json");
            if (!f.is_open()) {
                return; // Use defaults if no config
            }
            
            nlohmann::json data = nlohmann::json::parse(f);
            
            if (data.contains("debug")) {
                auto debug = data["debug"];
                if (debug.contains("renderDebug")) {
                    m_renderDebug = debug["renderDebug"].get<bool>();
                }
                if (debug.contains("shadowDebug")) {
                    m_shadowDebug = debug["shadowDebug"].get<bool>();
                }
            }
        } catch (const std::exception& e) {
            // Ignore config parsing errors, use defaults
        }
    }
    
    bool isRenderDebugEnabled() const { return m_renderDebug; }
    bool isShadowDebugEnabled() const { return m_shadowDebug; }
};

// Static member definition
DebugConfig* DebugConfig::instance = nullptr;