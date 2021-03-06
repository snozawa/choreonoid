/**
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BASE_SCENE_BAR_H_INCLUDED
#define CNOID_BASE_SCENE_BAR_H_INCLUDED

#include <cnoid/ToolBar>
#include "exportdecl.h"

namespace cnoid {

class SceneWidget;
class SceneBarImpl;

class CNOID_EXPORT SceneBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static SceneBar* instance();

    SceneWidget* targetSceneWidget();

protected:
    SceneBar();

private:
    SceneBarImpl* impl;
};
}

#endif
