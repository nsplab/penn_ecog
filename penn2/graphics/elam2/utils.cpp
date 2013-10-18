
#include "utils.h"

osg::Geode* createAxis()
{
    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec4Array* colors   = new osg::Vec4Array();

    vertices->push_back(osg::Vec3(-2.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(2.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, -2.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 2.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, -2.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 2.0f));

    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.5f, 0.5f, 1.0f, 1.0f));
    colors->push_back(osg::Vec4(0.5f, 0.5f, 1.0f, 1.0f));

    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 6));
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, false);

    geode->addDrawable(geometry);

    return geode;
}

int PowerToPixel(float mean_power)
/*******************************************************
 *This function graps the current value for the power, and using user defined bounds,
 *it maps it to the screen in the following way
 *for a screen:
 + - - - - - - - +   beta_2 = 50 => alpha_2
 |               |
 |               |
 |               |
 |               |
 +---------------+
 |               |
 |               | beta_1 = -37 => alpha_1
 |               |
 |               |
 + - - - - - - - + beta_0 = -50 => alpha_0
 *
 *Alpha is the power, and the equations map 2 regimes from beta to alpha_1
 *Regime for lower part of the screen:
 *
 *y_{position}=(beta_0-beta_1)*\frac{mean power}{alpha_0-alpha_1}+\frac{(alpha_0*beta_1-alpha_1*beta_0)}{(alpha_0-alpha_1)}
 *
 *Regime for the higher art of the screen
 *
 *y_{position}=meanpower*\frac{(beta_1-beta_2)}{(alpha_1-alpha_2)}+\frac{(beta_2*alpha_1-beta_1*alpha_2)}{(alpha_1-alpha_2)}
 *
 *Equations are formatted for Latex to see them prettier
 *********************************************************/
{
    //double alpha_0=2.3;
    //double alpha_2=1.5;
    //double alpha_1=1.8;
    double alpha_0 = 2.3;
    double alpha_1 = 1.8;
    double alpha_2 = 1.4;
    double alpha_sat;
    double beta_1=-37;
    double beta_2=37;
    double beta_0=-50;
    double y_position=0.0;
    if (mean_power>alpha_1){
                    y_position=(beta_0-beta_1)*mean_power/(alpha_0-alpha_1)+(alpha_0*beta_1-alpha_1*beta_0)/(alpha_0-alpha_1);
                   }
    alpha_sat=((50-beta_1)-(beta_2-beta_1)*alpha_1/(alpha_1-alpha_2))*((alpha_1-alpha_2)/(beta_1-beta_2));
    if (mean_power<=alpha_1)
    {
        y_position=y_position=mean_power*(beta_1-beta_2)/(alpha_1-alpha_2)+(beta_2*alpha_1-beta_1*alpha_2)/(alpha_1-alpha_2);
    }
    //double alpha_sat=(100-(beta_2-beta_1)*alpha_2/(alpha_1-alpha_2))*((alpha_1-alpha_2)/(beta_1-beta_2));
    if (mean_power<alpha_sat){
        y_position=50;
    }
    //redRect.y=5000;
    //bounds for the boxes, if the box is horizontal, it should be redRect.x
    if (y_position > 49)
        y_position = 50;
    else if (y_position < -49)
        y_position = -50;

    return int(y_position);
}

void themeMusic()
{
    for(;;) {
        FILE *pipe = popen( "play ./Angry_Birds_Theme.wav && echo done!", "r" );
        pclose(pipe);
    }
}

void tadaMusic()
{
   popen( "play ./tada.wav", "r" );
}

void dingMusic()
{
   popen( "play ./dingding.wav", "r" );
}

osg::Camera* createHUDCamera( double left, double right,
                              double bottom, double top )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER  );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix(
                osg::Matrix::ortho2D(left, right, bottom, top) );
    return camera.release();
}

osgText::Text* createText( const osg::Vec3& pos,
                           const std::string& content,float size, bool cfont,
                           osg::Vec4 tcolor)
{
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    if (!cfont)
        text->setFont( g_font.get() );
    else
        text->setFont( c_font.get() );
    text->setCharacterSize( size );
    text->setColor(tcolor);
    text->setAxisAlignment( osgText::TextBase::XY_PLANE );
    text->setPosition( pos );
    text->setText( content );
    return text.release();
}

bool GuiKeyboardEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
    case(osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON):
    {
        if (demoMode)
            openHand = true;

        break;
    }
    case(osgGA::GUIEventAdapter::KEYDOWN):
    {
        switch(ea.getKey())
        {
        case 'g':
            if (handState)
                return false;
            std::cout << " g key pressed" << std::endl;
            //graspObj(true, *(boneFinder), step);
            if (step >= GRASP_STEPS)
                step = 0;
            handState = true;
            return false;
            break;
        case 'r':
            if (!handState)
                return false;
            std::cout << " r key pressed" << std::endl;
            //graspObj(false, *(boneFinder), step);
            if (step >= GRASP_STEPS)
                step = 0;
            handState = false;

            return false;
            break;
        case 'q':
            ball_x -= 0.1;
            return false;
        case 'a':
            ball_x += 0.1;
            return false;

        case 'w':
            ball_y -= 0.1;
            return false;
        case 's':
            ball_y += 0.1;
            return false;

        case 'e':
            ball_z -= 0.1;
            return false;
        case 'd':
            ball_z += 0.1;
            return false;
        case osgGA::GUIEventAdapter::KEY_Space:
            pauseGame = !pauseGame;
            return true;


        default:
            return false;
        }
    }
    default:
        return false;
    }
}

void graspObj(bool grasp, BoneFinder& boneFinder, int step) {
    osg::Quat tmp_quat;
    if (grasp)
        tmp_quat.makeRotate((-35.0*0.01745329*(float(step)/GRASP_STEPS)), osg::Vec3(0.0f,0.0f,1.0f));
    else
        tmp_quat.makeRotate((-35.0*0.01745329*(float(GRASP_STEPS-step)/GRASP_STEPS)), osg::Vec3(0.0f,0.0f,1.0f));

    osg::Quat tmp_quat2;
    if (grasp)
        tmp_quat2.makeRotate((-55.0*0.01745329*(float(step)/GRASP_STEPS)), osg::Vec3(0.0f,0.0f,1.0f));
    else
        tmp_quat2.makeRotate((-55.0*0.01745329*(float(GRASP_STEPS-step)/GRASP_STEPS)), osg::Vec3(0.0f,0.0f,1.0f));

    Eigen::Matrix3d finger_mq[15];
    Eigen::Quaterniond qfingers[15];

    // index finger
    finger_mq[0] << 0.98412,0.17716,0.01103,-0.17739,0.9838,0.02595,-0.00626,-0.02749,0.9996;
    qfingers[0] = Eigen::Quaterniond(finger_mq[0]);

    finger_mq[1] << 1,0,0,0,1,0,0,0,1;
    qfingers[1] = Eigen::Quaterniond(finger_mq[1]);

    finger_mq[2] << 0.99915,0.04115,0.00256,-0.04115,0.99915,-0.00011,-0.00256,0,1;
    qfingers[2] = Eigen::Quaterniond(finger_mq[2]);

    // middle finger
    finger_mq[3] << 0.99347,0.11391,0.00709,-0.11375,0.99332,-0.01961,-0.00928,0.01868,0.99978;
    qfingers[3] = Eigen::Quaterniond(finger_mq[3]);

    finger_mq[4] << 1,0,0,0,1,0,0,0,1;
    qfingers[4] = Eigen::Quaterniond(finger_mq[4]);

    finger_mq[5] << 0.99915,0.04113,0.00256,-0.04113,0.99915,-0.00011,-0.00256,0,1;
    qfingers[5] = Eigen::Quaterniond(finger_mq[5]);

    // ring finger
    finger_mq[6] << 0.9961,0.08806,0.00548,-0.08807,0.99611,0.00264,-0.00523,-0.00311,0.99998;
    qfingers[6] = Eigen::Quaterniond(finger_mq[6]);

    finger_mq[7] << 1,0,0,0,1,0,0,0,1;
    qfingers[7] = Eigen::Quaterniond(finger_mq[7]);

    finger_mq[8] << 0.99915,0.04119,0.00256,-0.04119,0.99915,-0.00011,-0.00256,0,1;
    qfingers[8] = Eigen::Quaterniond(finger_mq[8]);

    // pinky finger
    finger_mq[9] << 0.99926,0.03845,0.00239,-0.03848,0.99915,0.01475,-0.00182,-0.01483,0.99989;
    qfingers[9] = Eigen::Quaterniond(finger_mq[9]);

    finger_mq[10] << 1,0,0,0,1,0,0,0,1;
    qfingers[10] = Eigen::Quaterniond(finger_mq[10]);

    finger_mq[11] << 0.99915,0.04115,0.00256,-0.04115,0.99915,-0.0001,-0.00256,0,1;
    qfingers[11] = Eigen::Quaterniond(finger_mq[11]);

    // index finger
    osgAnimation::UpdateBone* hand_update = new osgAnimation::UpdateBone("finger_index.01.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03736, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat2 * osg::Quat(qfingers[0].x(), qfingers[0].y(), qfingers[0].z(), qfingers[0].w())));
    boneFinder._bones[45]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.02.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03163, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[1].x(), qfingers[1].y(), qfingers[1].z(), qfingers[1].w())));
    boneFinder._bones[46]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.03.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03109, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[2].x(), qfingers[2].y(), qfingers[2].z(), qfingers[2].w())));
    boneFinder._bones[47]->setUpdateCallback(hand_update);

    // middle finger
    hand_update = new osgAnimation::UpdateBone("finger_index.01.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.04148, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat2 * osg::Quat(qfingers[3].x(), qfingers[3].y(), qfingers[3].z(), qfingers[3].w())));
    boneFinder._bones[52]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.02.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03313, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[4].x(), qfingers[4].y(), qfingers[4].z(), qfingers[4].w())));
    boneFinder._bones[53]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.03.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03257, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[5].x(), qfingers[5].y(), qfingers[5].z(), qfingers[5].w())));
    boneFinder._bones[54]->setUpdateCallback(hand_update);

    // ring finger
    hand_update = new osgAnimation::UpdateBone("finger_index.01.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.04275, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat2 * osg::Quat(qfingers[6].x(), qfingers[6].y(), qfingers[6].z(), qfingers[6].w())));
    boneFinder._bones[56]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.02.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03163, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[7].x(), qfingers[7].y(), qfingers[7].z(), qfingers[7].w())));
    boneFinder._bones[57]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.03.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.03109, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[8].x(), qfingers[8].y(), qfingers[8].z(), qfingers[8].w())));
    boneFinder._bones[58]->setUpdateCallback(hand_update);

    // pinky finger
    hand_update = new osgAnimation::UpdateBone("finger_index.01.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.04649, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat2 * osg::Quat(qfingers[9].x(), qfingers[9].y(), qfingers[9].z(), qfingers[9].w())));
    boneFinder._bones[60]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.02.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.02724, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[10].x(), qfingers[10].y(), qfingers[10].z(), qfingers[10].w())));
    boneFinder._bones[61]->setUpdateCallback(hand_update);

    hand_update = new osgAnimation::UpdateBone("finger_index.03.R");
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", osg::Vec3(0, 0.02677, 0)));
    hand_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate", tmp_quat * osg::Quat(qfingers[11].x(), qfingers[11].y(), qfingers[11].z(), qfingers[11].w())));
    boneFinder._bones[62]->setUpdateCallback(hand_update);

}


