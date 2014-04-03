/* Copyright (c) 2010 Mosalam Ebrahimi <m.ebrahimi@ieee.org>
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include "tweakbargui.h"

#include <iostream>

using namespace std;

void TW_CALL HideModel(void* clientData) {
    ((ExportState *)clientData)->hide_button = !((ExportState *)clientData)->hide_button;
}
void TW_CALL Pause(void* clientData) {
    ((ExportState *)clientData)->pause = !((ExportState *)clientData)->pause;
    ((ExportState *)clientData)->current_point = 0;

    if ((((ExportState *)clientData)->pause) == false) {
        ((ExportState *)clientData)->training = false;
    }
    //if (((ExportState *)clientData)->pause)
    //	((ExportState *)clientData)->status = "Paused";
    //else
    //	((ExportState *)clientData)->status = "Live";
}

void TW_CALL AddNewKF(void* clientData) {
    ((ExportState *)clientData)->new_kf = true;
}

void TW_CALL HideFeatures(void* clientData) {
    ((ExportState *)clientData)->hide_features = !((ExportState *)clientData)->hide_features;
}

void TW_CALL HideBorder(void* clientData) {
    ((ExportState *)clientData)->hide_border = !((ExportState *)clientData)->hide_border;
}

void TW_CALL HideAxes(void* clientData) {
    ((ExportState *)clientData)->hide_axes = !((ExportState *)clientData)->hide_axes;
}

void TW_CALL NewTarget(void* clientData) {
    ((ExportState *)clientData)->hide_features = !((ExportState *)clientData)->hide_features;
    ((ExportState *)clientData)->current_point = 0;
}

TweakBarEventCallback::TweakBarEventCallback(ExportState* s) : export_state(s)
{
    export_state->pause = false;
    export_state->state = false;
    export_state->hide = false;
    export_state->appear = false;
    export_state->corners_set = false;
    export_state->hide_button = false;
    export_state->hide_border = true;
    export_state->hidden_border = false;
    export_state->hide_features = true;
    export_state->hidden_features = false;
    export_state->hide_axes = true;
    export_state->hidden_axes = false;

    export_state->status = "Trial";

    export_state->coords = new osg::Vec3Array(5);
    export_state->upd_coords = new osg::Vec3Array(5);

    export_state->ratio = 1.414f;
    export_state->obj_scale = 1.0;

    export_state->focal_len = 370.0;

    export_state->new_kf = false;
    export_state->kfs = 0;

    export_state->current_point = 0;

    export_state->training = false;

    export_state->kf_match = -1;

    export_state->filter = true;

    current_point = 0;


    TwInit(TW_OPENGL, NULL);

    TwWindowSize(640, 480);

    toolsBar = TwNewBar("tools");
    TwDefine(" tools label='Helios' ");
    TwDefine(" GLOBAL help='The Helios Project\n'");
    TwDefine(" tools help='By clicking on the Pause/Resume button enter the \
                editing mode.' ");
    TwDefine(" tools size='170 220' ");
    TwDefine(" tools movable=true ");
    TwDefine(" tools resizable=true ");
    TwDefine(" tools refresh=0.1 ");
    TwDefine(" GLOBAL fontresizable=false ");
    TwDefine(" GLOBAL contained=true ");
//    TwAddButton(toolsBar, "Show/Hide Model", HideModel, s, NULL);
//    TwAddButton(toolsBar, "Pause/Resume", Pause, s, NULL);
//    TwAddButton(toolsBar, "Add new Keyframe", AddNewKF, s, NULL);
//    TwAddVarRO(toolsBar, "#KFs", TW_TYPE_INT32, &(export_state->kfs), " label='# of keyframes' ");
//    TwAddVarRW(toolsBar, "Status", TW_TYPE_STDSTRING, &(export_state->status), NULL);
//    TwAddVarRW(toolsBar, "Ratio", TW_TYPE_FLOAT, &(export_state->ratio), " min=0.1 max=4 step=0.01 ");
//    TwAddVarRW(toolsBar, "Object Scale", TW_TYPE_FLOAT, &(export_state->obj_scale), " min=0.5 max=1.5 step=0.1 ");
//    TwAddSeparator(toolsBar, NULL, " ");
//    TwAddButton(toolsBar, "Show/Hide Border", HideBorder, s, NULL);
//    TwAddButton(toolsBar, "Show/Hide Axes", HideAxes, s, NULL);
//    TwAddButton(toolsBar, "Show/Hide Features", HideFeatures, s, NULL);
//    TwAddVarRO(toolsBar, "KF matched", TW_TYPE_INT32, &(export_state->kf_match), " label='# of keyframes' ");
//    TwAddVarRW(toolsBar, "AB Filter", TW_TYPE_BOOLCPP, &(export_state->filter), " ");
    //TwAddVarRW(toolsBar, "Focal Len (pixels)", TW_TYPE_FLOAT, &(export_state->focal_len), " min=150 max=450 step=1 ");
    //TwAddButton(toolsBar, "New Target", NewTarget, s, NULL);
    TwAddVarRW(toolsBar, "Status", TW_TYPE_STDSTRING, &(export_state->status), NULL);

}


bool TweakBarEventCallback::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
{
        float x = ea.getX();
        float y = ea.getY();

        bool handled = false;
        switch ( ea.getEventType() ) {
        case(osgGA::GUIEventAdapter::DRAG):
        case ( osgGA::GUIEventAdapter::MOVE ) : {
             handled = TwMouseMotion ( x, ea.getWindowHeight() - y );
             break;
        }
        case (osgGA::GUIEventAdapter::PUSH): {
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
                    handled = TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
                    if (!handled && (export_state->pause) && (current_point < 4)) {

                        if (export_state->current_point < 4)
                            export_state->current_point += 1;

                        (*(export_state->coords))[export_state->current_point-1] =
                                                            osg::Vec3(x, y, 0);
                        (*(export_state->upd_coords))[export_state->current_point-1] =
                                                            osg::Vec3(x, y, 0);
                        if (export_state->current_point == 1) {
                            (*(export_state->coords))[4] =	osg::Vec3(x, y, 0);
                            (*(export_state->upd_coords))[4] =	osg::Vec3(x, y, 0);
                        }

                        if ((export_state->current_point == 4) && (export_state->kfs == 0) )
                            export_state->corners_set = true;
                        handled = true;
                    }
            }
            else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
                    handled = TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_MIDDLE);
            else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
                    handled = TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT);
            break;
        }
        case(osgGA::GUIEventAdapter::RELEASE): {
            if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                    handled = TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
            else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
                    handled = TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_MIDDLE);
            else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
                    handled = TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
            break;
        }
        case(osgGA::GUIEventAdapter::RESIZE): {
            handled = TwWindowSize(ea.getWindowWidth(), ea.getWindowHeight());
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN): {
            switch(ea.getKey()) {
                case 'p':
                    export_state->pause = !export_state->pause;
                    current_point = 0;
                    export_state->corners_set = false;
                    break;
                case 'h':
                    export_state->hide = true;
                    export_state->appear = false;
                    break;
                case 'a':
                    export_state->appear = true;
                    export_state->hide = false;
                    break;
            }
        }
        default:
                break;
        }

        return handled;
}

TweakBarDrawable::TweakBarDrawable()
{
        setUseDisplayList(false);
}


TweakBarDrawable::~TweakBarDrawable()
{
        TwTerminate();
}

void TweakBarDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
        TwDraw();
}

osg::Camera* TweakBarDrawable::createHUD(int width, int height)
{
        osg::Camera* camera = new osg::Camera;

        camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));

        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());

        camera->setClearMask(GL_DEPTH_BUFFER_BIT);

        camera->setRenderOrder(osg::Camera::POST_RENDER, 10000);

        camera->setAllowEventFocus(false);

        return camera;
}

