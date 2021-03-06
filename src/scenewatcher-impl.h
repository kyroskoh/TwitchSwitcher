// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <list>

#include <twitchsw/scenewatcher.h>
#include <twitchsw/refs.h>

#include <obs.hpp>

namespace twitchsw {

// Model
// SceneWatcherImpl
//     - Holds pointers to Scene objects, but does not retain them
// Scene
//     - Holds pointers to obs_source_t and obs_sceneitem_t, but does not retain
//       them.
//
// Held (but not retained) items are removed when signals are received from libobs,
// preventing memory leaks.
//
// All SceneWatcher activity occurs on the main thread, in response to libobs signals.
class SceneWatcherImpl;
class Scene : public RefCounted<Scene> {
public:
    explicit Scene(SceneWatcherImpl* impl, obs_source_t* scene);
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    ~Scene();

    SceneWatcherImpl* impl() const { return m_impl; }
    obs_source_t* source() const { return m_source; }
    obs_sceneitem_t* item() const { return m_item; }

    static bool isTwitchSceneItem(obs_sceneitem_t* item);

    void updateIfNeeded(bool force = false);

private:
    // SceneWatcherImpl's lifetime should always be longer than Scene instances,
    // so this is not a RefPtr.
    SceneWatcherImpl* m_impl;
    obs_source_t* m_source;
    obs_sceneitem_t* m_item;

    void connectSignalHandlers();
    void disconnectSignalHandlers();
    static obs_sceneitem_t* takeFirstTwitchSceneItem(obs_source_t* source, obs_sceneitem_t* ignore = nullptr);
    static bool takeFirstTwitchSceneItemProc(obs_scene_t* scene, obs_sceneitem_t* item, void* param);

    // Signal handlers:
    // void item_add(ptr scene : obs_scene_t, ptr item : obs_sourceitem_t)
    static void onAddSceneItem(void* userdata, calldata_t* calldata);

    // void item_remove(ptr scene : obs_scene_t, ptr item : obs_sourceitem_t)
    static void onRemoveSceneItem(void* userdata, calldata_t* calldata);

    // void transition_start(ptr source : obs_source_t)
    static void onTransitionStart(void* userdata, calldata_t* calldata);

    // void source_show(ptr source : obs_source_t)
    static void onShow(void* userdata, calldata_t* calldata);

    // void activate(ptr source : obs_source_t)
    static void onActivate(void* userdata, calldata_t* calldata);
};

class SceneWatcherImpl {
public:
    SceneWatcherImpl();
    ~SceneWatcherImpl();

    void addScene(obs_source_t* scene);
    void removeScene(obs_source_t* scene);

    bool isStreaming();

    void setCurrentScene(PassRefPtr<Scene> scene) {
        m_currentScene = scene;
    }

    void scanForStreamingOutputs();
    void scanForStreamingServices();
    bool getTwitchCredentials(String& key);

    RefPtr<Scene> findScene(obs_source_t* source);

private:
    std::list<RefPtr<Scene>> m_scenes;
    RefPtr<Scene> m_currentScene;
    OBSWeakOutput m_streamingOutput;
    OBSWeakService m_streamingService;

    void connectSignalHandlers();
    void disconnectSignalHandlers();

    // Signal handlers:
    // void source_create(ptr source : obs_source_t)
    // void source_load(ptr source : obs_source_t)
    static void addSourceIfNeeded(void* userdata, calldata_t* calldata);

    // void source_remove(ptr source : obs_source_t)
    // void source_destroy(ptr source : obs_source_t)
    static void removeSourceIfNeeded(void* userdata, calldata_t* calldata);

    // void start(ptr output : obs_output_t)
    static void onStartStreaming(void* userdata, calldata_t* calldata);

    // Helpers
    static bool isTwitchStream(obs_output_t* output);
    static bool isTwitchStream(obs_service_t* service);

    static bool scanForStreamingOutputsProc(void* param, obs_output_t* output);
    static bool scanForStreamingServicesProc(void* param, obs_service_t* service);
};

}  // namespace twitchsw
