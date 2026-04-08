#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/EditorUI.hpp>

using namespace geode::prelude;

namespace tools {
    void connectObjects(EditorUI* editorUI, CCObject* sender);
    void fillObjects(EditorUI* editorUI, CCObject* sender);
    void replaceObjectType(EditorUI* editorUI, CCObject* sender);
    void circleObjects(EditorUI* editorUI, CCObject* sender);
    void connectTrigger(EditorUI* editorUI, CCObject* sender);
    void disconnectTrigger(EditorUI* editorUI, CCObject* sender);
}
