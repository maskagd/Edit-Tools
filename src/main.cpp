#include <Geode/Geode.hpp>
#include "tools.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

static CCMenuItemSpriteExtra* createToolButton(
    char const* frameName,
    float scale,
    geode::Function<void()> callback,
    char const* background = "GJ_button_01.png"
) {
    auto* icon = CCSprite::create(frameName);
    if (!icon) {
        return nullptr;
    }

    auto* sprite = ButtonSprite::create(icon, 42, true, 42.f, background, 1.2f);
    sprite->setScale(scale);

    return CCMenuItemExt::createSpriteExtra(sprite, [callback = std::move(callback)](auto) mutable {
        callback();
    });
}

static CCMenuItemSpriteExtra* createInfoButton(char const* infoText) {
    auto* icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    if (!icon) {
        return nullptr;
    }

    icon->setScale(0.78f);
    return CCMenuItemExt::createSpriteExtra(icon, [infoText](auto) {
        FLAlertLayer::create("Edit Tools", infoText, "Ok")->show();
    });
}

class ToolsPopup : public Popup {
protected:
    EditorUI* m_editorUI = nullptr;

    void addToolButton(
        char const* frameName,
        cocos2d::CCPoint position,
        char const* infoText,
        geode::Function<void()> callback
    ) {
        auto* button = createToolButton(frameName, 0.96f, std::move(callback));
        if (!button) {
            return;
        }

        m_buttonMenu->addChildAtPosition(button, Anchor::Center, position);

        auto* infoButton = createInfoButton(infoText);
        if (!infoButton) {
            return;
        }

        m_buttonMenu->addChildAtPosition(infoButton, Anchor::Center, ccp(position.x + 22.f, position.y + 20.f));
    }

    bool init(EditorUI* editorUI) {
        if (!Popup::init(270.f, 205.f, "GJ_square02.png")) {
            return false;
        }

        m_noElasticity = true;
        m_editorUI = editorUI;
        this->setTitle("Edit Tools");
        m_closeBtn->setVisible(false);
        m_closeBtn->setEnabled(false);

        constexpr float columnSpacing = 62.f;
        constexpr float rowSpacing = 52.f;
        float startX = -93.f;
        float topRowY = 34.f;
        float bottomRowY = topRowY - rowSpacing;

        this->addToolButton(
            "connect.png"_spr,
            ccp(startX + columnSpacing * 0.f, topRowY),
            "Select <cy>two objects</c>. Creates copies between them. Which object's visual properties are used depends on the tool settings",
            [this]() {
            this->onClose(nullptr);
            tools::connectObjects(m_editorUI, nullptr);
        });
        this->addToolButton(
            "fill.png"_spr,
            ccp(startX + columnSpacing * 1.f, topRowY),
            "Select <cy>two corner objects</c>. Fills the area between them with copies using the current fill settings",
            [this]() {
            this->onClose(nullptr);
            tools::fillObjects(m_editorUI, nullptr);
        });
        this->addToolButton(
            "replace.png"_spr,
            ccp(startX + columnSpacing * 2.f, topRowY),
            "Select <cy>two objects</c>. Replaces the first object's ID with the second object's ID. With the setting enabled, it swaps both objects",
            [this]() {
            this->onClose(nullptr);
            tools::replaceObjectType(m_editorUI, nullptr);
        });
        this->addToolButton(
            "replicate.png"_spr,
            ccp(startX + columnSpacing * 3.f, topRowY),
            "Select the <cy>main object first</c>, then any number of other objects. All following objects will be recreated with the main object's ID",
            [this]() {
            this->onClose(nullptr);
            tools::replicateObject(m_editorUI, nullptr);
        });
        this->addToolButton(
            "circle.png"_spr,
            ccp(startX + columnSpacing * 0.f, bottomRowY),
            "Select <cy>two objects</c>. One is the center and the other is the source placed on the circle. The tool creates a ring of copies around the center",
            [this]() {
            this->onClose(nullptr);
            tools::circleObjects(m_editorUI, nullptr);
        });
        this->addToolButton(
            "grouped.png"_spr,
            ccp(startX + columnSpacing * 1.f, bottomRowY),
            "If you select <cy>triggers and objects</c>, the tool gives the objects a free group and assigns that group to the triggers. If you select <cy>only triggers</c>, the first selected trigger becomes the activator and all later triggers are grouped to its target group",
            [this]() {
            this->onClose(nullptr);
            tools::connectTrigger(m_editorUI, nullptr);
        });
        this->addToolButton(
            "ungrouped.png"_spr,
            ccp(startX + columnSpacing * 2.f, bottomRowY),
            "Select <cy>triggers and objects</c> to remove the objects from the selected triggers' target groups. If you select <cy>only triggers</c>, the first trigger is used as the source trigger and the rest are treated like objects for the disconnect",
            [this]() {
            this->onClose(nullptr);
            tools::disconnectTrigger(m_editorUI, nullptr);
        });
        this->addToolButton(
            "spawned.png"_spr,
            ccp(startX + columnSpacing * 3.f, bottomRowY),
            "Select the <cy>triggers you want to wrap into a spawn setup</c>. The first trigger becomes the template for a new spawn trigger, its <cy>groups are moved to that spawn trigger</c>, the selected triggers keep only one new free group",
            [this]() {
            this->onClose(nullptr);
            tools::spawnedTrigger(m_editorUI, nullptr);
        });

        constexpr float bottomControlsY = 24.f;
        constexpr float settingsInsetX = 26.f;

        if (auto* settingsButton = createToolButton("settings.png"_spr, 0.72f, []() {
            geode::openSettingsPopup(Mod::get(), false);
        }, "GJ_button_04.png")) {
            m_buttonMenu->addChildAtPosition(
                settingsButton,
                Anchor::BottomLeft,
                ccp(settingsInsetX, bottomControlsY)
            );
        }

        auto* okSprite = ButtonSprite::create("OK", "goldFont.fnt", "GJ_button_01.png", 1.5f);
        okSprite->setScale(1.0f);
        auto* okButton = CCMenuItemExt::createSpriteExtra(okSprite, [this](auto) {
            this->onClose(nullptr);
        });
        m_buttonMenu->addChildAtPosition(
            okButton,
            Anchor::Bottom,
            ccp(0.f, bottomControlsY)
        );

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

    listenForKeybindSettingPresses("keybind-replicate-object", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::replicateObject(editorUI, nullptr);
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

    listenForKeybindSettingPresses("keybind-spawned-trigger", [](auto const&, bool down, bool repeat, double) {
        if (!down || repeat) {
            return false;
        }

        auto* editorUI = EditorUI::get();
        if (!editorUI) {
            return false;
        }

        tools::spawnedTrigger(editorUI, nullptr);
        return true;
    });
}

class $modify(EditTools, EditorUI) {
    void createMoveMenu() {
        registerKeybinds();
        EditorUI::createMoveMenu();

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

        if (Mod::get()->getSettingValue<bool>("toolbar-show-replicate")) {
            if (auto* button = createToolButton("replicate.png"_spr, 1.0f, [this]() {
                tools::replicateObject(this, nullptr);
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

        if (Mod::get()->getSettingValue<bool>("toolbar-show-spawned")) {
            if (auto* button = createToolButton("spawned.png"_spr, 1.0f, [this]() {
                tools::spawnedTrigger(this, nullptr);
            })) {
                m_editButtonBar->m_buttonArray->addObject(button);
            }
        }

        auto rows = GameManager::sharedState()->getIntGameVariable("0049");
        auto cols = GameManager::sharedState()->getIntGameVariable("0050");
        m_editButtonBar->reloadItems(rows, cols);
    }
};
