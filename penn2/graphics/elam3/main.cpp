/*Copyright (c) 2013, Mosalam Ebrahimi <m.ebrahimi@ieee.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

// coordinate systems:
// + the padding is added by elam3
//#####################################################################
//## the coordinate system in the supervisor:
//##
//##              Y ^ [0,1,0]
//##                |
//##                |
//##                |__________> X [1, 0, 0]
//##               /
//##              /
//##             /
//##         Z  v [0, 0, 1]
//##
//## the coordinate system in the graphics module:
//##
//##
//##        [0,0,W/2] Z ^   ^ X [W, 0, 0]
//##                    |  /
//##                    | /
//## [0, W, 0] Y <______|/
//##
//##
//#####################################################################
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

#include <zmq.hpp>

#include "utils.h"

#include "inversekinematics.h"

using namespace std;
using namespace zmq;

extern bool pauseGame;


int main(int argc, char *argv[])
{
    ////////////////////////////////
    // PREPARING GRAPHICS based on openscenegraph (osg)
    // http://www.openscenegraph.org/index.php/download-section/code-repositories
    // OSG Version 3.0.1, used with Ubuntu Linux 13.04
    // 1. Create instances of all graphics objects defined above
    // 2. Create instance of the arm object which also implements forward kinematics.
    // 3. Render arm on screen.
    ////////////////////////////////
    
    /* define variable root which is the main Group of scene graph, which organizes all
     graphical elements on the screen into a group for example, rendering from a specific
     camera angle can be executed on this root
     */
    osg::ref_ptr<osg::Group> root = new osg::Group();

    //create a cartoon node, set its properties; this will define the appearance of subsequent objects on the screen
    osg::ref_ptr<osgFX::Cartoon> cartoon = new osgFX::Cartoon;
    cartoon->setOutlineColor(osg::Vec4(0.0, 0.0, 0.0, 1.0f));
    cartoon->setOutlineLineWidth(2.0);

    // load upper arm model, downloaded from http://www.blendswap.com/ and edit in Blender to extract only the arm of a robot
    osg::ref_ptr<osg::Node> upperArm = osgDB::readNodeFile("../upper_arm.3ds");

    // load forearm model (we had excluded the .3ds hand model for now)
    osg::ref_ptr<osg::Node> foreArm = osgDB::readNodeFile("../fore_arm.3ds");

    // declare an object that sets transparency and associate it with the upperArm and foreArm; this trick is needed to get proper transparency effects on the arm.
    MakeTransparent mkTransparent(0.6);
    upperArm->accept(mkTransparent);
    foreArm->accept(mkTransparent);

    //make osg position and attitude nodes for rotation and translation related to the upper arm
    osg::ref_ptr<osg::PositionAttitudeTransform> patUpperArm = new osg::PositionAttitudeTransform();
    patUpperArm->setPosition(osg::Vec3(0,0,-2.0));
    patUpperArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(3.141592, osg::Vec3d(0,0,1)));
    //make osg position and attitude nodes for rotation and translation related to the forearm
    osg::ref_ptr<osg::PositionAttitudeTransform> patForeArm = new osg::PositionAttitudeTransform();
    patForeArm->setPosition(osg::Vec3(0,0,-2.0));
    patForeArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(0, osg::Vec3d(0,0,1)));


    //code for debugging makes a small sphere at the position of the elbow
    osg::ref_ptr<osg::ShapeDrawable> elbowPoint = new osg::ShapeDrawable;
    elbowPoint->setShape( new osg::Sphere(osg::Vec3(0.0, 0.0, 0.0), 0.1f) );
    osg::ref_ptr<osg::Geode> elbowPointGeode = new osg::Geode;
    elbowPointGeode->addDrawable(elbowPoint.get());
    patForeArm->addChild(elbowPointGeode);

    //code for debugging makes a small sphere at the position of the shoulder
    osg::ref_ptr<osg::ShapeDrawable> shoulderPoint = new osg::ShapeDrawable;
    shoulderPoint->setShape( new osg::Sphere(osg::Vec3(0.0, 0.0, 0.0), 0.1f) );
    osg::ref_ptr<osg::Geode> shoulderPointGeode = new osg::Geode;
    shoulderPointGeode->addDrawable(shoulderPoint.get());
    patUpperArm->addChild(shoulderPointGeode);


    patUpperArm->addChild(upperArm);

    osg::ref_ptr<osg::ShapeDrawable> sphereHand = new osg::ShapeDrawable;
    sphereHand->setShape( new osg::Sphere(osg::Vec3(-3.8, -0.107, 0.03077), 0.5f) );
    osg::ref_ptr<osg::Geode> sphere = new osg::Geode;
    sphere->addDrawable(sphereHand.get());
    patForeArm->addChild(sphere);

    patForeArm->addChild(foreArm);
    patUpperArm->addChild(patForeArm);

    osg::ref_ptr<TransparencyNode> fxNode = new TransparencyNode;
    fxNode->addChild(patUpperArm);
    //cartoon->addChild(fxNode);
    root->addChild(fxNode);

    root->addChild(createMeshCube());

    osg::DisplaySettings::instance()->setNumMultiSamples(4); // TODO: causes segfault for bryan (6/17)
    osgViewer::Viewer visor;
    visor.setSceneData(root);
    visor.setUpViewInWindow(0,0,1000,1000,0);
    osgViewer::Viewer::Windows windows;
    visor.getWindows(windows);
    //visor.realize();
    windows[0]->setWindowName("3D Env");

    visor.setCameraManipulator(new osgGA::TrackballManipulator);
    visor.getCamera()->setClearColor(osg::Vec4(0.9f,0.9f,0.9f,1.0f));

    cout<<"camera repositioned"<<endl;

    // set initial position of the camera
    osg::Matrixd m(1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0.0, 0.0, 0.0, 1);
    m = m.rotate(3.141592/2.0, 1,0,0) * m;
    m = m.rotate(-3.141592/2.0, 0,1,0) * m;
    m = m.rotate(-3.141592/6.0, 1,0,0) * m;
    m = m.translate(-2.5,1.0,11.5) * m;

    visor.getCameraManipulator()->setByMatrix(m);

    
    
    // ik 1 set initial positions and orientaions
    Eigen::Vector3f elbowPos(-4.92282, -0.1, 0.0);

    patForeArm->setPosition(osg::Vec3d(elbowPos(0),elbowPos(1),elbowPos(2)));

    Eigen::Vector3d IKtarget(0, 0, 0);

    osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable;
    shape1->setShape( new osg::Box(osg::Vec3(IKtarget(0), IKtarget(1), IKtarget(2)), 0.7f) );
    osg::ref_ptr<osg::Geode> box = new osg::Geode;
    box->addDrawable(shape1.get());
    //root->addChild(box);

    osg::ref_ptr<osg::ShapeDrawable> shape2 = new osg::ShapeDrawable;
    shape2->setShape( new osg::Box(osg::Vec3(0, 0, 0), 0.5f) );
    osg::ref_ptr<osg::Geode> box2 = new osg::Geode;
    box2->addDrawable(shape2.get());
    osg::ref_ptr<osg::PositionAttitudeTransform> patBox = new osg::PositionAttitudeTransform();
    patBox->addChild(box2);
    //root->addChild(patBox);
    //patBox->setPosition(osg::Vec3d(-wristPos(0),wristPos(1),wristPos(2)));

    //root->addChild(box);

    Eigen::Quaternion<float> shoulderQuatO(patUpperArm->getAttitude().w(), patUpperArm->getAttitude().x(), patUpperArm->getAttitude().y(), patUpperArm->getAttitude().z());
    Eigen::Quaternion<float> elbowQuatO(patForeArm->getAttitude().w(), patForeArm->getAttitude().x(), patForeArm->getAttitude().y(), patForeArm->getAttitude().z());

    Eigen::Quaternion<float> shoulderQuat  = shoulderQuatO;
    Eigen::Quaternion<float> elbowQuat = elbowQuatO;

    // ikf
    Eigen::Vector3d currentWristPos;
    //SolveArmInvKinematics(target, currentWristPos, shoulderQuat, elbowQuat, patUpperArm);

    patBox->setPosition(osg::Vec3d(currentWristPos(0),currentWristPos(1),currentWristPos(2)));

    //patUpperArm->setAttitude(osg::Quat(shoulderQuat.x(),shoulderQuat.y(),shoulderQuat.z(),shoulderQuat.w()));
    //patForeArm->setAttitude(osg::Quat(elbowQuat.x(),elbowQuat.y(),elbowQuat.z(),elbowQuat.w()));



    context_t context(1);
    socket_t subscriber(context, ZMQ_REQ);
    subscriber.connect("ipc:///tmp/graphics.pipe");
    //subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::message_t state_msg;

    osg::Box* currentCube = new osg::Box(osg::Vec3(2.5,0.5,-2.0), 5, 0.5, 0.5);
    osg::ShapeDrawable* currentCubeDrawable = new osg::ShapeDrawable(currentCube);
    currentCubeDrawable->setColor(osg::Vec4(0.2,0.6,1.0,0.7));
    currentCubeDrawable->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    currentCubeDrawable->getOrCreateStateSet()->setRenderBinDetails( 12, "RenderBin" );
    currentCubeDrawable->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
    currentCubeDrawable->setUseDisplayList(false);
    osg::Geode* basicShapesGeode = new osg::Geode();
    basicShapesGeode->addDrawable(currentCubeDrawable);
    //root->addChild(basicShapesGeode);
    //cartoon->addChild(basicShapesGeode); // TODO: could make more compatible by using root->addChild(basicShapesGeode) instead
    root->addChild(basicShapesGeode);


    osg::Box* nextCube = new osg::Box(osg::Vec3(2.5,0.5,-2.0), 5, 0.5, 0.5);
    osg::ShapeDrawable* nextCubeDrawable = new osg::ShapeDrawable(nextCube);
    nextCubeDrawable->setColor(osg::Vec4(0.2,0.6,1.0,0.7));
    nextCubeDrawable->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    nextCubeDrawable->getOrCreateStateSet()->setRenderBinDetails( 12, "RenderBin" );
    nextCubeDrawable->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );    nextCubeDrawable->setUseDisplayList(false);
    osg::Geode* nextShapesGeode = new osg::Geode();
    nextShapesGeode->addDrawable(nextCubeDrawable);
    //root->addChild(nextShapesGeode);
    //cartoon->addChild(nextShapesGeode);
    root->addChild(nextShapesGeode);
    //root->addChild(cartoon);

    GuiKeyboardEventHandler* guiEventHandler = new GuiKeyboardEventHandler();
    visor.addEventHandler(guiEventHandler);

    osg::ref_ptr<osg::MatrixTransform> parent = new osg::MatrixTransform;
    parent->setMatrix(osg::Matrix::rotate(osg::PI_2, osg::Y_AXIS)
                      * osg::Matrix::translate(0.0f,0.0f,0.0f));

    osgParticle::ParticleSystem* fire = createFireParticles(parent.get() );

    osg::ref_ptr<osgParticle::ParticleSystemUpdater> updater =
    new osgParticle::ParticleSystemUpdater;
    updater->addParticleSystem(fire);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( fire );

    osg::ref_ptr<osg::Switch> fireSwitch = new osg::Switch;
    fireSwitch->addChild(parent.get(), false);
    fireSwitch->addChild(updater.get(), true);
    fireSwitch->addChild(geode.get(), true);

    root->addChild(fireSwitch);

    //fireSwitch->setAllChildrenOff();

    osg::ref_ptr<osgText::Font> g_font = osgText::readFontFile("../Ubuntu-B.ttf");

    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    osg::ref_ptr<osgText::Text> scoreText = createText(
                                            osg::Vec3(10.0f, 10.0f, 0.0f),
                                            "Score: 0", 80.0f, g_font);
    textGeode->addDrawable(scoreText);

    osg::Camera* camera = createHUDCamera(0, 1024, 0, 700);
    camera->addChild( textGeode.get() );

    osg::ref_ptr<osg::Geode> text2Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score2Text = createText2(
                                            osg::Vec3(210.0f, 700-75, 0.0f),
                                            "Score/Sec", 10.0f, g_font);
    score2Text->setRotation(osg::Quat(osg::PI/2.0,osg::Vec3(0,0,1)));

    text2Geode->addDrawable(score2Text);
    camera->addChild( text2Geode.get() );

    osg::ref_ptr<osg::Geode> text21Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score21Text = createText2(
                                            osg::Vec3(210.0f, 700-99, 0.0f),
                                            "0", 10.0f, g_font);
    score21Text->setRotation(osg::Quat(osg::PI/2.0,osg::Vec3(0,0,1)));

    text21Geode->addDrawable(score21Text);
    camera->addChild( text21Geode.get() );

    osg::ref_ptr<osg::Geode> text22Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score22Text = createText2(
                                            osg::Vec3(210.0f, 700-6, 0.0f),
                                            "1", 10.0f, g_font);
    score22Text->setRotation(osg::Quat(osg::PI/2.0,osg::Vec3(0,0,1)));

    text22Geode->addDrawable(score22Text);
    camera->addChild( text22Geode.get() );

    osg::ref_ptr<osg::Geode> text3Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score3Text = createText2(
                                            osg::Vec3(75.0f, 700-110, 0.0f),
                                            "time (min)", 10.0f, g_font);

    text3Geode->addDrawable(score3Text);
    camera->addChild( text3Geode.get() );

    osg::ref_ptr<osg::Geode> text4Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score4Text = createText2(
                                            osg::Vec3(1.0f, 700-110, 0.0f),
                                            "20", 10.0f, g_font);

    text4Geode->addDrawable(score4Text);
    camera->addChild( text4Geode.get() );

    osg::ref_ptr<osg::Geode> text5Geode = new osg::Geode;
    osg::ref_ptr<osgText::Text> score5Text = createText2(
                                            osg::Vec3(195.0f, 700-110, 0.0f),
                                            "1", 10.0f, g_font);

    text5Geode->addDrawable(score5Text);
    camera->addChild( text5Geode.get() );

    osg::Vec3Array* plotArray;
    float scaleY = 100.0;
    camera->addChild(createPlotDecoration(200.0, 100.0));
    camera->addChild(createPlot(200.0, scaleY, 19, &plotArray));

    camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF );
    root->addChild( camera );

    SolveArmInvKinematics(IKtarget, currentWristPos, shoulderQuat, elbowQuat, patUpperArm, patForeArm,
                          sphere, elbowPointGeode);

    float score = 0;
    float prevScore = 0;
    while ( !visor.done() ){

        visor.frame();

       cout<<"send"<<endl;
        if (pauseGame) {
            subscriber.send("p", 2);
        } else {
            subscriber.send("c", 2);
        }
        cout<<"sent"<<endl;


        cout<<"before subscriber.recv"<<endl;
        subscriber.recv(&state_msg);
        cout<<"after subscriber.recv"<<endl;


        //cout<<"received"<<endl;
        string state_str(((char *)state_msg.data()));
        //cout<<"message: "<<state_str<<endl;
        stringstream ss;
        ss.str(state_str);
        char type;
        ss>>type;
        float currentBlockLen = 0.0;
        float currentBlockX, currentBlockY, currentBlockZ;
        float nextBlockLen = 0.0;
        float nextBlockStart = 0.0;
        float nextBlockX, nextBlockY, nextBlockZ;
        float handPos[3];
        float tmp;
        float blockWidth;
        float scorePlot;

        float timeScale = 0.4;


        if (type == 'B') {
            cout<<"B message: "<<state_str<<endl;
            ss>>currentBlockLen;
            ss>>nextBlockStart;
            ss>>nextBlockLen;
            ss>>currentBlockX;
            ss>>currentBlockY;
            ss>>currentBlockZ;
            ss>>nextBlockX;
            ss>>nextBlockY;
            ss>>nextBlockZ;
            ss>>blockWidth;
            ss>>handPos[0];
            ss>>handPos[1];
            ss>>handPos[2];
            ss>>tmp;
            ss>>tmp;
            ss>>tmp;
            ss>>score;
            ss>>scorePlot;

            cout<<"*************************"<<endl;
            cout<<"blockWidth "<<blockWidth<<endl;

            cout << "currentBlockX "<<currentBlockX<<endl;
            currentBlockX = 4.5 + currentBlockX * -4.0;
            currentBlockY = -2.0 + currentBlockY * 4.5;
            currentBlockZ = 4.5 + currentBlockZ * -4.0;
            cout << "currentBlockX "<<currentBlockX<<endl;

            nextBlockX = 4.5 + nextBlockX * -4.0;
            nextBlockY = -2.0 + nextBlockY * 4.5;
            nextBlockZ = 4.5 + nextBlockZ * -4.0;

            cout<<"nextBlockX "<<nextBlockX<<endl;

            if (scorePlot > -0.5) {
                scoreText->setText(string("Score: ") + to_string((float)scorePlot).substr(0, 4));
            }

            if (score > prevScore) {
                fireSwitch->setChildValue(parent.get(), true);
                prevScore = score;
                parent->setMatrix(osg::Matrix::rotate(osg::PI_2, osg::Y_AXIS)
                                  * osg::Matrix::translate(currentBlockZ, currentBlockX, currentBlockY));
            } else {
                fireSwitch->setChildValue(parent.get(), false);
            }

            if (scorePlot > -0.5) {
                for (unsigned i=0; i<(plotArray->size()-1); i++){
                    (*plotArray)[i][1] = (*plotArray)[i+1][1];
                }
                (*plotArray)[plotArray->size()-1][1] = 700 + scaleY*(scorePlot-1.0);
            }

            currentCube->setCenter(osg::Vec3(5.0 - (currentBlockLen)/2.0 * timeScale, currentBlockX, currentBlockY));
            currentCube->setHalfLengths(osg::Vec3((currentBlockLen)/2.0 * timeScale,blockWidth*0.025, blockWidth*0.025));
            nextCube->setCenter(osg::Vec3(5.0 - nextBlockStart * timeScale - (nextBlockLen)/2.0 * timeScale, nextBlockX, nextBlockY));
            nextCube->setHalfLengths(osg::Vec3((nextBlockLen)/2.0 * timeScale,blockWidth*0.025, blockWidth*0.025));


            cout<<"handPos[0] "<<handPos[0]<<endl;
            cout<<"handPos[1] "<<handPos[1]<<endl;
            cout<<"handPos[2] "<<handPos[2]<<endl;

            handPos[0] =  4.5 + handPos[0] * -4.0;
            handPos[1] = -2.0 + handPos[1] * 4.5;
            handPos[2] =  4.5 + handPos[2] * -4.0;

            cout<<"handPos[0] "<<handPos[0]<<endl;
            cout<<"handPos[1] "<<handPos[1]<<endl;
            cout<<"handPos[2] "<<handPos[2]<<endl;

            IKtarget(0) = handPos[2];
            IKtarget(1) = handPos[0];
            IKtarget(2) = handPos[1];


            //target(2) = 0.0;

            //shoulderQuat = shoulderQuatO;
            //elbowQuat = elbowQuatO;
            cout<<"solve IK"<<endl;
            float error = SolveArmInvKinematics((Eigen::Vector3d&)IKtarget, (Eigen::Vector3d&)currentWristPos, shoulderQuat, elbowQuat, patUpperArm, patForeArm,
                                  sphere, elbowPointGeode);
            if (isnan(error)) {
                return 1;
            }
            cout<<"IK error: "<<error<<endl;
            cout<<"solved IK"<<endl;
        }
        fprintf(stderr, "argc: %d != 1?\n", argc);
        if (argc != 1) {
            break;
        }

    }

    return 0;
}

