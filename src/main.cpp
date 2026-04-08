#include <Geode/Geode.hpp>
#include "tools.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

static CCMenuItemSpriteExtra* createToolButton(
    char const* frameName,
    float scale,
    geode::Function<void()> callback
) {
    auto* icon = CCSprite::create(frameName);
    if (!icon) {
        return nullptr;
    }

    auto* sprite = ButtonSprite::create(icon, 42, true, 42.f, "GJ_button_01.png", 1.2f);
    sprite->setScale(scale);

    return CCMenuItemExt::createSpriteExtra(sprite, [callback = std::move(callback)](auto) mutable {
        callback();
    });
}

class ToolsPopup : public Popup {
protected:
    EditorUI* m_editorUI = nullptr;

    void addToolButton(char const* frameName, cocos2d::CCPoint position, geode::Function<void()> callback) {
        auto* button = createToolButton(frameName, 0.85f, std::move(callback));
        if (!button) {
            return;
        }

        m_buttonMenu->addChildAtPosition(button, Anchor::Center, position);
    }

    bool init(EditorUI* editorUI) {
        if (!Popup::init(290.f, 150.f, "GJ_square05.png")) {
            return false;
        }

        m_noElasticity = true;
        m_editorUI = editorUI;
        this->setTitle("Edit Tools");
        m_closeBtn->setVisible(false);
        m_closeBtn->setEnabled(false);

        float spacing = 36.f;
        float startX = -spacing * 3.f;
        float rowY = 8.f;

        this->addToolButton("connect.png"_spr, ccp(startX + spacing * 0.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::connectObjects(m_editorUI, nullptr);
        });
        this->addToolButton("fill.png"_spr, ccp(startX + spacing * 1.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::fillObjects(m_editorUI, nullptr);
        });
        this->addToolButton("replace.png"_spr, ccp(startX + spacing * 2.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::replaceObjectType(m_editorUI, nullptr);
        });
        this->addToolButton("circle.png"_spr, ccp(startX + spacing * 3.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::circleObjects(m_editorUI, nullptr);
        });
        this->addToolButton("grouped.png"_spr, ccp(startX + spacing * 4.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::connectTrigger(m_editorUI, nullptr);
        });
        this->addToolButton("ungrouped.png"_spr, ccp(startX + spacing * 5.f, rowY), [this]() {
            this->onClose(nullptr);
            tools::disconnectTrigger(m_editorUI, nullptr);
        });
        this->addToolButton("settings.png"_spr, ccp(startX + spacing * 6.f, rowY), []() {
            geode::openSettingsPopup(Mod::get(), false);
        });

        auto* okSprite = ButtonSprite::create("OK", "goldFont.fnt", "GJ_button_01.png", 1.5f);
        okSprite->setScale(1.0f);
        auto* okButton = CCMenuItemExt::createSpriteExtra(okSprite, [this](auto) {
            this->onClose(nullptr);
        });
        m_buttonMenu->addChildAtPosition(okButton, Anchor::Center, ccp(0.f, -42.f));

        return true;
    }

public:
    static ToolsPopup* create(EditorUI* editorUI) {
        auto* ret = new ToolsPopup();
        if (ret && ret->init(editorUI)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};

static void openToolsPopup(EditorUI* editorUI) {
    auto* popup = ToolsPopup::create(editorUI);
    if (popup) {
        popup->show();
    }
}

static void registerKeybinds() {
    static bool registered = false;
    if (registered) {
        return;
    }

    registered = true;

    listenForKeybindSettingPresses("keybind-open-popup", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        openToolsPopup(editorUI);
        return true;
    });

    listenForKeybindSettingPresses("keybind-connect", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::connectObjects(editorUI, nullptr);
        return true;
    });

    listenForKeybindSettingPresses("keybind-fill", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::fillObjects(editorUI, nullptr);
        return true;
    });

    listenForKeybindSettingPresses("keybind-replace-type", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::replaceObjectType(editorUI, nullptr);
        return true;
    });

    listenForKeybindSettingPresses("keybind-circle", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::circleObjects(editorUI, nullptr);
        return true;
    });

    listenForKeybindSettingPresses("keybind-connect-trigger", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::connectTrigger(editorUI, nullptr);
        return true;
    });

    listenForKeybindSettingPresses("keybind-disconnect-trigger", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::disconnectTrigger(editorUI, nullptr);
        return true;
    });
}

class $modify(EditTools, EditorUI) {
    void createMoveMenu() {
        registerKeybinds();
        EditorUI::createMoveMenu();

        auto addToolbarButton = [this](char const* frameName, geode::Function<void()> callback) {
            auto* button = createToolButton(frameName, 1.0f, std::move(callback));
            if (!button) {
                return;
            }

            m_editButtonBar->m_buttonArray->addObject(button);
        };

        if (auto* button = createToolButton("menu.png"_spr, 1.0f, [this]() {
            openToolsPopup(this);
        })) {
            m_editButtonBar->m_buttonArray->addObject(button);
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-connect")) {
            if (auto* button = createToolButton("connect.png"_spr, 1.0f, [this]() {
                tools::connectObjects(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-fill")) {
            if (auto* button = createToolButton("fill.png"_spr, 1.0f, [this]() {
                tools::fillObjects(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-replace-type")) {
            if (auto* button = createToolButton("replace.png"_spr, 1.0f, [this]() {
                tools::replaceObjectType(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-circle")) {
            if (auto* button = createToolButton("circle.png"_spr, 1.0f, [this]() {
                tools::circleObjects(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-connect-trigger")) {
            if (auto* button = createToolButton("grouped.png"_spr, 1.0f, [this]() {
                tools::connectTrigger(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        if (Mod::get()->getSettingValue<bool>("toolbar-show-disconnect-trigger")) {
            if (auto* button = createToolButton("ungrouped.png"_spr, 1.0f, [this]() {
                tools::disconnectTrigger(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        auto rows = GameManager::sharedState()->getIntGameVariable("0049");
        auto cols = GameManager::sharedState()->getIntGameVariable("0050");
        m_editButtonBar->reloadItems(rows, cols);
    }
};
