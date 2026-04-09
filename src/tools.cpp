#include "tools.hpp"

#include <cmath>
#include <numbers>
#include <algorithm>
#include <string>
#include <vector>
#include <Geode/c++stl/gdstdlib.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

namespace tools {
    namespace {
        constexpr float kFloatTolerance = 0.001f;

        bool nearlyEqual(float first, float second) {
            return std::abs(first - second) < kFloatTolerance;
        }

        bool nearlyEqual(cocos2d::CCPoint first, cocos2d::CCPoint second) {
            return nearlyEqual(first.x, second.x) && nearlyEqual(first.y, second.y);
        }

        float alignToGrid(float value) {
            float gridSize = Mod::get()->getSettingValue<float>("grid-size");
            return std::round(value / gridSize) * gridSize - gridSize / 2.0f;
        }

        bool hasPosition(std::vector<cocos2d::CCPoint> const& positions, cocos2d::CCPoint const& target) {
            return std::ranges::any_of(positions, [&](cocos2d::CCPoint const& position) {
                return nearlyEqual(position, target);
            });
        }

        void copyObjectProperties(GameObject* object, GameObject* source) {
            object->setScaleX(source->m_scaleX);
            object->setScaleY(source->m_scaleY);
            object->setRotation(source->getObjectRotation());
            object->setFlipX(source->m_isFlipX);
            object->setFlipY(source->m_isFlipY);
        }

        std::string replaceObjectIDInSaveString(std::string saveString, int objectID) {
            if (!saveString.empty() && saveString.back() == ';') {
                saveString.pop_back();
            }

            std::vector<std::string> parts;
            size_t start = 0;
            while (start <= saveString.size()) {
                size_t comma = saveString.find(',', start);
                if (comma == std::string::npos) {
                    parts.push_back(saveString.substr(start));
                    break;
                }

                parts.push_back(saveString.substr(start, comma - start));
                start = comma + 1;
            }

            for (size_t i = 0; i + 1 < parts.size(); i += 2) {
                if (parts[i] == "1") {
                    parts[i + 1] = std::to_string(objectID);
                    break;
                }
            }

            std::string result;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) {
                    result += ",";
                }
                result += parts[i];
            }
            result += ";";
            return result;
        }

        GameObject* pasteObjectFromSaveString(EditorUI* editorUI, std::string const& saveString) {
            auto* pastedObjects = editorUI->pasteObjects(saveString, true, false);
            if (!pastedObjects || pastedObjects->count() == 0) {
                return nullptr;
            }

            return static_cast<GameObject*>(pastedObjects->objectAtIndex(0));
        }

        void selectCreatedObjects(EditorUI* editorUI, GameObject* first, cocos2d::CCArray* createdObjects, GameObject* second) {
            if (!createdObjects || createdObjects->count() == 0) {
                return;
            }

            auto* allSelectedObjects = cocos2d::CCArray::create();
            allSelectedObjects->addObject(first);
            for (unsigned int i = 0; i < createdObjects->count(); ++i) {
                allSelectedObjects->addObject(createdObjects->objectAtIndex(i));
            }
            allSelectedObjects->addObject(second);

            editorUI->deselectAll();
            editorUI->selectObjects(allSelectedObjects, false);
            editorUI->updateButtons();
        }
    }

    void connectObjects(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() != 2) {
            FLAlertLayer::create("Edit Tools", "Select the <cy>two</c> objects you want to connect", "Ok")->show();
            return;
        }

        auto* first = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
        auto* second = static_cast<GameObject*>(selectedObjs->objectAtIndex(1));
        bool useFirst = Mod::get()->getSettingValue<bool>("connect-first-properties");
        bool gridAlignment = Mod::get()->getSettingValue<bool>("connect-grid-alignment");
        auto* source = useFirst ? first : second;
        auto firstPos = first->getPosition();
        auto secondPos = second->getPosition();

        if (firstPos == secondPos) {
            FLAlertLayer::create("Edit Tools", "The selected objects are at the <cy>same position</c>", "Ok")->show();
            return;
        }

        auto delta = ccpSub(secondPos, firstPos);
        auto distance = ccpLength(delta);
        float step = Mod::get()->getSettingValue<float>("grid-size");
        if (!gridAlignment) {
            auto bounds = source->boundingBox();
            step = std::abs(delta.x) >= std::abs(delta.y) ? bounds.size.width : bounds.size.height;
        }
        if (step <= 0.0f) {
            return;
        }

        int fillCount = static_cast<int>(std::floor(distance / step)) - 1;
        if (fillCount <= 0) {
            return;
        }

        auto* editorLayer = LevelEditorLayer::get();
        auto* filledObjects = cocos2d::CCArray::create();
        std::vector<cocos2d::CCPoint> createdPositions;
        for (int i = 1; i <= fillCount; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(fillCount + 1);
            auto position = ccpAdd(firstPos, ccpMult(delta, t));
            if (gridAlignment) {
                position.x = alignToGrid(position.x);
                position.y = alignToGrid(position.y);
            }

            if (nearlyEqual(position, firstPos) || nearlyEqual(position, secondPos) || hasPosition(createdPositions, position)) {
                continue;
            }

            auto* object = editorLayer->createObject(source->m_objectID, position, false);
            if (!object) {
                continue;
            }

            copyObjectProperties(object, source);
            filledObjects->addObject(object);
            createdPositions.push_back(position);
        }

        selectCreatedObjects(editorUI, first, filledObjects, second);
    }

    void fillObjects(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() != 2) {
            FLAlertLayer::create("Edit Tools", "Select the <cy>two</c> corner objects you want to fill", "Ok")->show();
            return;
        }

        auto* first = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
        auto* second = static_cast<GameObject*>(selectedObjs->objectAtIndex(1));
        bool useFirst = Mod::get()->getSettingValue<bool>("fill-first-properties");
        auto* source = useFirst ? first : second;
        auto firstPos = first->getPosition();
        auto secondPos = second->getPosition();

        if (firstPos == secondPos) {
            FLAlertLayer::create("Edit Tools", "The selected objects are at the <cy>same position</c>", "Ok")->show();
            return;
        }

        bool gridAlignment = Mod::get()->getSettingValue<bool>("fill-grid-alignment");
        bool outlineOnly = Mod::get()->getSettingValue<bool>("fill-outline-only");
        float gridSize = Mod::get()->getSettingValue<float>("grid-size");

        float startX = std::min(firstPos.x, secondPos.x);
        float endX = std::max(firstPos.x, secondPos.x);
        float startY = std::min(firstPos.y, secondPos.y);
        float endY = std::max(firstPos.y, secondPos.y);

        float stepX = source->boundingBox().size.width;
        float stepY = source->boundingBox().size.height;

        if (gridAlignment) {
            startX = alignToGrid(startX);
            endX = alignToGrid(endX);
            startY = alignToGrid(startY);
            endY = alignToGrid(endY);
            stepX = gridSize;
            stepY = gridSize;
        }

        if (stepX <= 0.0f || stepY <= 0.0f) {
            return;
        }

        auto* editorLayer = LevelEditorLayer::get();
        auto* filledObjects = cocos2d::CCArray::create();

        for (float x = startX; x <= endX + 0.001f; x += stepX) {
            for (float y = startY; y <= endY + 0.001f; y += stepY) {
                if (outlineOnly) {
                    bool onLeftEdge = nearlyEqual(x, startX);
                    bool onRightEdge = nearlyEqual(x, endX);
                    bool onBottomEdge = nearlyEqual(y, startY);
                    bool onTopEdge = nearlyEqual(y, endY);
                    if (!onLeftEdge && !onRightEdge && !onBottomEdge && !onTopEdge) {
                        continue;
                    }
                }

                auto position = ccp(x, y);
                if (nearlyEqual(position, firstPos)) {
                    continue;
                }
                if (nearlyEqual(position, secondPos)) {
                    continue;
                }

                auto* object = editorLayer->createObject(source->m_objectID, position, false);
                if (!object) {
                    continue;
                }

                copyObjectProperties(object, source);
                filledObjects->addObject(object);
            }
        }

        selectCreatedObjects(editorUI, first, filledObjects, second);
    }

    void replaceObjectType(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() != 2) {
            FLAlertLayer::create("Edit Tools", "Select the <cy>two</c> objects for type replace", "Ok")->show();
            return;
        }

        auto* first = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
        auto* second = static_cast<GameObject*>(selectedObjs->objectAtIndex(1));
        bool swapBoth = Mod::get()->getSettingValue<bool>("replace-swap-both");

        auto* editorLayer = LevelEditorLayer::get();
        auto firstSaveString = replaceObjectIDInSaveString(first->getSaveString(editorLayer), second->m_objectID);
        std::string secondSaveString;
        if (swapBoth) {
            secondSaveString = replaceObjectIDInSaveString(second->getSaveString(editorLayer), first->m_objectID);
        }

        auto* firstReplacement = pasteObjectFromSaveString(editorUI, firstSaveString);
        if (!firstReplacement) {
            return;
        }

        GameObject* secondReplacement = second;
        if (swapBoth) {
            secondReplacement = pasteObjectFromSaveString(editorUI, secondSaveString);
            if (!secondReplacement) {
                editorLayer->removeObject(firstReplacement, false);
                return;
            }
        }

        editorLayer->removeObject(first, false);
        if (swapBoth) {
            editorLayer->removeObject(second, false);
        }

        auto* selectedAfterReplace = cocos2d::CCArray::create();
        selectedAfterReplace->addObject(firstReplacement);
        selectedAfterReplace->addObject(secondReplacement);

        editorUI->deselectAll();
        editorUI->selectObjects(selectedAfterReplace, false);
        editorUI->updateButtons();
    }

    void circleObjects(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() != 2) {
            FLAlertLayer::create("Edit Tools", "Select the <cy>center</c> object and one <cy>circle</c> object", "Ok")->show();
            return;
        }

        auto* first = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
        auto* second = static_cast<GameObject*>(selectedObjs->objectAtIndex(1));
        bool firstIsCenter = Mod::get()->getSettingValue<bool>("circle-first-center");
        auto* center = firstIsCenter ? first : second;
        auto* source = firstIsCenter ? second : first;

        auto centerPos = center->getPosition();
        auto sourcePos = source->getPosition();
        auto delta = ccpSub(sourcePos, centerPos);
        float radius = ccpLength(delta);
        if (radius <= 0.0f) {
            FLAlertLayer::create("Edit Tools", "The circle radius is <cy>zero</c>", "Ok")->show();
            return;
        }

        float spacing = Mod::get()->getSettingValue<float>("circle-spacing");
        if (spacing <= 0.0f) {
            auto bounds = source->boundingBox();
            spacing = std::max(bounds.size.width, bounds.size.height);
        }
        if (spacing <= 0.0f) {
            return;
        }

        float circumference = 2.0f * static_cast<float>(std::numbers::pi) * radius;
        int objectCount = std::max(1, static_cast<int>(std::round(circumference / spacing)));
        if (objectCount <= 1) {
            return;
        }

        bool gridAlignment = Mod::get()->getSettingValue<bool>("circle-grid-alignment");
        bool rotateObjects = Mod::get()->getSettingValue<bool>("circle-rotate-objects");
        float startAngle = std::atan2(delta.y, delta.x);
        float angleStep = 2.0f * static_cast<float>(std::numbers::pi) / static_cast<float>(objectCount);
        float sourceRotation = source->getObjectRotation();

        auto* editorLayer = LevelEditorLayer::get();
        auto* createdObjects = cocos2d::CCArray::create();
        std::vector<cocos2d::CCPoint> createdPositions;
        for (int i = 1; i < objectCount; ++i) {
            float angle = startAngle + angleStep * static_cast<float>(i);
            auto position = ccp(
                centerPos.x + std::cos(angle) * radius,
                centerPos.y + std::sin(angle) * radius
            );

            if (gridAlignment) {
                position.x = alignToGrid(position.x);
                position.y = alignToGrid(position.y);
            }

            if (nearlyEqual(position, centerPos)) {
                continue;
            }
            if (nearlyEqual(position, sourcePos) || hasPosition(createdPositions, position)) {
                continue;
            }

            auto* object = editorLayer->createObject(source->m_objectID, position, false);
            if (!object) {
                continue;
            }

            copyObjectProperties(object, source);
            if (rotateObjects) {
                float angleOffset = (angle - startAngle) * 180.0f / static_cast<float>(std::numbers::pi);
                object->setRotation(sourceRotation - angleOffset);
            }
            createdObjects->addObject(object);
            createdPositions.push_back(position);
        }

        selectCreatedObjects(editorUI, first, createdObjects, second);
    }   

    void connectTrigger(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() < 2) {
            FLAlertLayer::create("Edit Tools", "Select <cy>trigger</c> and <cy>objects</c>", "Ok")->show();
            return;
        }

        std::vector<EffectGameObject*> effectObjects;
        std::vector<GameObject*> gameObjects;

        for (unsigned int i = 0; i < selectedObjs->count(); ++i) {
            auto* object = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (object->m_isTrigger) {
                effectObjects.push_back(static_cast<EffectGameObject*>(object));
            }
            else {
                gameObjects.push_back(object);
            }
        }

        if (effectObjects.empty() || gameObjects.empty()) {
            FLAlertLayer::create("Edit Tools", "Need at least <cy>one trigger</c> and <cy>one object</c>", "Ok")->show();
            return;
        }

        auto* editorLayer = LevelEditorLayer::get();
        gd::unordered_set<int> excludedGroups;
        int groupID = editorLayer->getNextFreeGroupID(excludedGroups);

        for (auto* object : gameObjects) {
            editorLayer->addToGroup(object, groupID, false);
            if (!object->belongsToGroup(groupID)) {
                object->addToGroup(groupID);
            }
            editorLayer->updateObjectSection(object);
            LevelEditorLayer::updateObjectLabel(object);
        }

        for (auto* effectObject : effectObjects) {
            effectObject->m_targetGroupID = groupID;
            editorLayer->updateObjectSection(effectObject);
            LevelEditorLayer::updateObjectLabel(effectObject);
        }

        editorUI->updateButtons();
    }

    void disconnectTrigger(EditorUI* editorUI, CCObject* sender) {
        static_cast<void>(sender);

        if (!editorUI) {
            return;
        }

        auto* selectedObjs = editorUI->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() < 2) {
            FLAlertLayer::create("Edit Tools", "Select <cy>trigger</c> and <cy>objects</c>", "Ok")->show();
            return;
        }

        std::vector<EffectGameObject*> effectObjects;
        std::vector<GameObject*> gameObjects;

        for (unsigned int i = 0; i < selectedObjs->count(); ++i) {
            auto* object = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (object->m_isTrigger) {
                effectObjects.push_back(static_cast<EffectGameObject*>(object));
            }
            else {
                gameObjects.push_back(object);
            }
        }

        if (effectObjects.empty() || gameObjects.empty()) {
            FLAlertLayer::create("Edit Tools", "Need at least <cy>one trigger</c> and <cy>one object</c>", "Ok")->show();
            return;
        }

        gd::unordered_set<int> targetGroups;
        for (auto* effectObject : effectObjects) {
            if (effectObject->m_targetGroupID > 0) {
                targetGroups.insert(effectObject->m_targetGroupID);
            }
        }

        if (targetGroups.empty()) {
            FLAlertLayer::create("Edit Tools", "Selected triggers have no <cy>target group</c>", "Ok")->show();
            return;
        }

        auto* editorLayer = LevelEditorLayer::get();
        for (auto* object : gameObjects) {
            for (int groupID : targetGroups) {
                editorLayer->removeFromGroup(object, groupID);
                if (object->belongsToGroup(groupID)) {
                    object->removeFromGroup(groupID);
                }
            }
            editorLayer->updateObjectSection(object);
            LevelEditorLayer::updateObjectLabel(object);
        }

        if (Mod::get()->getSettingValue<bool>("disconnect-clear-target-group")) {
            for (auto* effectObject : effectObjects) {
                effectObject->m_targetGroupID = 0;
                editorLayer->updateObjectSection(effectObject);
                LevelEditorLayer::updateObjectLabel(effectObject);
            }
        }

        editorUI->updateButtons();
    }
}
