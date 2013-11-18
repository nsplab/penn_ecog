#include <osg/io_utils>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Bone>
#include <osgAnimation/StackedTransform>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedRotateAxisElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/UpdateBone>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osgAnimation/AnimationManagerBase>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgAnimation/Export>
#include <osgAnimation/UpdateMatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/ComputeBoundsVisitor>
#include <osgGA/GUIEventHandler>
#include <osgDB/ReadFile>
#include <osgText/Font>
#include <osgText/Text>
#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Canvas>

#include <fstream>
#include <iomanip>

#include <sstream>
#include <stdio.h>
#include <ctime>

#include <iostream>
#include <iterator>

#include <zmq.hpp>

#include <thread>

#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/kd.h>

#include <time.h>
#include <chrono> // timing

#include "utils.h"
#include "hud_Element.h"
#include "hud_Element_circle.h"
#include "textBox.h"

#include "GetPot.h"

using namespace std;
using namespace zmq;
using namespace chrono;

float roll,pitch,yaw, roll2,pitch2,yaw2, roll3,pitch3,yaw3;

bool subLevelFlag = false;

extern bool openHand;

extern float ball_x;
extern float ball_y;
extern float ball_z;

extern bool pauseGame;

extern osg::ref_ptr<osgText::Font> g_font;

extern osg::ref_ptr<osgText::Font> c_font;


int main(int argc, char** argv)
{
    string cfgFile("../elam2.cfg");

    // check if config exists
    ifstream testFile(cfgFile.c_str());
    if (! testFile.good()) {
        cout<<"Could not open the config file: "<<cfgFile<<endl;
        return 1;
    } else {
        testFile.close();
    }

    // parse config file
    GetPot ifile(cfgFile.c_str(), "#", "\n");
    ifile.print();

    int dimension = ifile("dimension", 2);
    int numWaypoints = ifile("numberOfWaypoint", 1);
    int sublevelPerLevel = ifile("numberOfSublevelsPerLevel", 2);
    int trialPerSublevel = ifile("numberOfTrialsPerSublevel", 4);
    int forcedBreaks = ifile("forceHavingBreaks", 4);
    int showDuplicateWindow = ifile("showDuplicateWindow", 4);

    ofstream descFile("description.txt");
    time_t rawtime;
    time(&rawtime);
    descFile<<"Date/Time: "<<ctime(&rawtime)<<endl;
    descFile<<"Dimension: "<<dimension<<endl;
    descFile<<"Number of waypoints: "<<numWaypoints<<endl;
    descFile<<"Number of sublevels per level: "<<sublevelPerLevel<<endl;
    descFile<<"Number of trials per sublevel: "<<trialPerSublevel<<endl;
    descFile.close();

    vector<FILE*> eventFiles(9);
    eventFiles[0] = fopen("e1", "wb");
    eventFiles[1] = fopen("e2", "wb");
    eventFiles[2] = fopen("e3", "wb");
    eventFiles[3] = fopen("e4", "wb");
    eventFiles[4] = fopen("e5", "wb");
    eventFiles[5] = fopen("e6", "wb");
    eventFiles[6] = fopen("e7", "wb");
    eventFiles[7] = fopen("e8", "wb");
    eventFiles[8] = fopen("e9", "wb");

    cout<<"num waypoints "<<numWaypoints<<endl;
    int currentWaypoint = -1;
    if (numWaypoints > 0)
        currentWaypoint = 0;

    // root of scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group();

    // load arm model
    osg::ref_ptr<osg::Node> tmodel = osgDB::readNodeFile("../bare_hand_Scene.osgt");
    // couldn't load the model
    if ( !tmodel ) return 1;

    // add axes and arm to scene graph
    root->addChild(createAxis());

    osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform();

    osg::ref_ptr<osg::PositionAttitudeTransform> pat2 = new osg::PositionAttitudeTransform();
    pat2->setAttitude(osg::Quat(-3.141592/2.0, osg::Vec3d(1,0,0)) * osg::Quat(3.141592/2.0, osg::Vec3d(0,1,0)) * osg::Quat(3.141592/4.0, osg::Vec3d(0,0,1)));
    pat2->setPosition(osg::Vec3f(-2.2, 1.2, -11.0));

    pat2->addChild(tmodel);
    osg::ref_ptr<osg::MatrixTransform> model =  new osg::MatrixTransform;
    model->preMult(osg::Matrix::translate(0.0f, 0.0f, 0.0f) *
    osg::Matrix::scale(-1.0f, 1.0f, 1.0f) *
    osg::Matrix::translate(0.0f, 0.0f, 0.0f) );
    model->addChild( pat2.get() );

    pat->addChild(model);
    root->addChild(pat);

    // list of bones in the model
    BoneFinder boneFinder;
    model->accept(boneFinder);

    for (int i=0; i<boneFinder._bones.size(); i++) {
        std::cout<<boneFinder._bones[i]->getName()<<std::endl;
    }

    // scene viewer
    osg::DisplaySettings::instance()->setNumMultiSamples(2);
    osgViewer::Viewer visor;
    visor.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    visor.setSceneData(root);
    visor.setCameraManipulator(new osgGA::TrackballManipulator);

    GuiKeyboardEventHandler* guiEventHandler = new GuiKeyboardEventHandler(true);
    guiEventHandler->boneFinder = &boneFinder;
    visor.addEventHandler(guiEventHandler);
    guiEventHandler->step = 0;
    guiEventHandler->handState = false;

    //visor.realize();
    visor.setUpViewInWindow(0,0,800,600,0);
    osgViewer::Viewer::Windows windows;
    visor.getWindows(windows);
    windows[0]->setWindowName("3D Env");

    osgViewer::Viewer visor2;
    if (showDuplicateWindow == 1) {
        visor2.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
        visor2.setSceneData(root);
        visor2.setCameraManipulator(new osgGA::TrackballManipulator);

        visor2.setUpViewInWindow(0,0,800,600,0);
        visor2.getWindows(windows);
        windows[0]->setWindowName("3D Env Dbg");
    }

    // HUD

    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> scoreTextGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> targetTextGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> sublTextGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> TrialLeftGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> pauseTextGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> summaryTextGeode = new osg::Geode;
    osg::ref_ptr<osg::Geode> timeLeftTextGeode = new osg::Geode;

    osg::ref_ptr<osg::Camera> camera = createHUDCamera(0, 1920, 0, 1200);
    //camera->addChild( textGeode.get() );
    camera->addChild( scoreTextGeode.get() );
    //camera->addChild( targetTextGeode.get() );
    //camera->addChild( sublTextGeode.get() );
    //camera->addChild( TrialLeftGeode.get() );
    //camera->addChild( pauseTextGeode.get() );
    camera->getOrCreateStateSet()->setMode(
                GL_LIGHTING, osg::StateAttribute::OFF );

    HUD_Element * hud_element = new HUD_Element();
    hud_element->setElementPosition(50, 50);
    hud_element->setElementSize(1920-100, 1200-100);
    hud_element->setElementColor(visor.getCamera()->getClearColor()[0]-0.2, visor.getCamera()->getClearColor()[1]-0.2, visor.getCamera()->getClearColor()[2]-0.2, 0.9);

    osg::Switch* switchHUD = new osg::Switch();

    summaryTextGeode.get()->getOrCreateStateSet()->setRenderBinDetails(11, "DepthSortedBin");
    timeLeftTextGeode.get()->getOrCreateStateSet()->setRenderBinDetails(12, "DepthSortedBin");

    switchHUD->addChild(hud_element->getGeode(),false);
    switchHUD->addChild(summaryTextGeode.get(), false);
    switchHUD->addChild(timeLeftTextGeode.get(), false);

    camera->addChild(switchHUD);

    osg::Sphere* sphere = new osg::Sphere( osg::Vec3(0.0,0.0,0.0), 0.7);
    osg::ShapeDrawable* sphereDrawable = new osg::ShapeDrawable(sphere);
    sphereDrawable->setColor(osg::Vec4(1.0, 0.5, 0.6, 1.0));
    sphereDrawable->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    osg::Geode* sphereGeode = new osg::Geode();
    osg::ref_ptr<osg::PositionAttitudeTransform> sph_trans = new osg::PositionAttitudeTransform();
    sph_trans->setScale(osg::Vec3d(1.0,1.0,1.0));
    //osg::Vec3d sph_pos(-3.0, -5.0, 4.0);
    osg::Vec3d sph_pos(-11, -1.6388, 0.75597);
    sph_trans->setPosition(sph_pos);
    sph_trans->setAttitude(osg::Quat(0, osg::Vec3d(0,0,0)));
    sph_trans->addChild(sphereGeode);
    root->addChild(sph_trans);
    sphereGeode->addDrawable(sphereDrawable);


    osg::Box* box = new osg::Box(osg::Vec3(0.0,0.0,0.0), 1.6);
    osg::ShapeDrawable* boxDrawable = new osg::ShapeDrawable(box);
    boxDrawable->setColor(osg::Vec4(0, 0.9, 0, 0.2));
    boxDrawable->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    osg::Geode* boxGeode = new osg::Geode();
    osg::ref_ptr<osg::PositionAttitudeTransform> box_trans = new osg::PositionAttitudeTransform();
    box_trans->setScale(osg::Vec3d(1.0,1.0,1.0));
    osg::Vec3d box_pos(0.0, 0.0, 0);
    box_trans->setPosition(box_pos);
    box_trans->setAttitude(osg::Quat(0, osg::Vec3d(0.0, 0.0, 0.0)));
    box_trans->addChild(boxGeode);
    root->addChild(box_trans);
    boxGeode->addDrawable(boxDrawable);

    vector<osg::ref_ptr<osg::PositionAttitudeTransform> > wPoints(numWaypoints);
    vector<osg::ref_ptr<osg::Box> > wBoxes(numWaypoints);
    vector<osg::ref_ptr<osg::ShapeDrawable> > wDrawables(numWaypoints);
    vector<osg::ref_ptr<osg::Geode> > wGeodes(numWaypoints);

    for (size_t i=0; i<numWaypoints; i++) {
        wBoxes[i] = new osg::Box(osg::Vec3(0.0,0.0,0.0), 1.6);
        wDrawables[i] = new osg::ShapeDrawable(wBoxes[i]);
        wDrawables[i]->setColor(osg::Vec4(0, 0.5, 0.9, 0.2));
        wDrawables[i]->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

        wGeodes[i] = new osg::Geode();
        wGeodes[i]->addDrawable(wDrawables[i]);

        wPoints[i] = new osg::PositionAttitudeTransform();
        wPoints[i]->setScale(osg::Vec3d(0.8,0.8,0.8));

        double x = (double)rand()/(RAND_MAX)* 14.0 - 7.0;
        double y = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
        double z = (double)rand()/(RAND_MAX)* 10.0 - 5;

        wPoints[i]->setPosition(osg::Vec3d(x, y, z));
        wPoints[i]->setAttitude(osg::Quat(0, osg::Vec3d(0.0, 0.0, 0.0)));
        wPoints[i]->addChild(wGeodes[i]);
        root->addChild(wPoints[i]);
    }


    root->addChild(camera);

    // set initial position of the camera
    visor.getCameraManipulator()->setByMatrix(osg::Matrixd(-1, 0, 0, 0,
                                                           0, 0, 1, 0,
                                                           0, 1, 0, 0,
                                                           0.0, 35, 0.0, 1 ));
    if (showDuplicateWindow)
        visor2.getCameraManipulator()->setByMatrix(osg::Matrixd(-1, 0, 0, 0,
                                                            0, 0, 1, 0,
                                                            0, 1, 0, 0,
                                                            0.0, 35, 0.0, 1 ));



    auto start = high_resolution_clock::now();

    int score = 0;


    context_t context(1);
    socket_t subscriber(context, ZMQ_SUB);
    uint64_t hwm = 1;
    subscriber.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
    subscriber.connect("ipc:///tmp/graphics.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::message_t state_msg;

    size_t level = 1;
    size_t subLevel = 1;
    size_t tmpTrial = 1;
    size_t paused = 0;

    osgViewer::Viewer::Windows sWindows;
    visor.getWindows(sWindows);
    sWindows[0]->setCursor(osgViewer::GraphicsWindow::NoCursor);


    while ( !visor.done()){


        paused = (size_t) pauseGame;

        if (pauseGame) {
            switchHUD->setValue(0, true);
            switchHUD->setValue(1, true);
            pauseTextGeode->removeDrawables(0);
            pauseTextGeode->addDrawable( createText(
                                             osg::Vec3(650.0f, 1040.0f, 0.0f),
                                             "PAUSED",
                                             95.0f, false)
                                         );
            ostringstream summary_ostr;
            summary_ostr<<"Summary: "<<score;
            summaryTextGeode->removeDrawables(0);
            summaryTextGeode->addDrawable( createText(
                                               osg::Vec3(500.0f, 500.0f, 0.0f),
                                               summary_ostr.str(),
                                               55.0f)
                                           );
            continue;
        } else {
            pauseTextGeode->removeDrawables(0);
            switchHUD->setValue(0, false);
            switchHUD->setValue(1, false);
        }

        // update next frame
        visor.frame();
        if (showDuplicateWindow)
            visor2.frame();

        subscriber.recv(&state_msg);

        string state_str(((char *)state_msg.data()));
        stringstream ss;
        ss.str(state_str);

        float handX=0.0, handY=0.0, handZ=0.0;
        float ballX=0.0, ballY=0.0, ballZ=0.0;
        float boxX=0.0, boxY=0.0, boxZ=0.0;
        ss>>handX>>handZ>>handY;
        ss>>ballX>>ballZ>>ballY;
        ss>>boxX>>boxZ>>boxY;

        int closedHand = 0;
        ss>>closedHand;

        ss>>score;

        ss>>level;
        ss>>subLevel;


        //pat->setAttitude(osg::Quat(-3.141592/2.0, osg::Vec3d(1,0,0)) * osg::Quat(3.141592/2.0, osg::Vec3d(0,1,0)) * osg::Quat(3.141592/4.0, osg::Vec3d(0,0,1)));
        pat->setPosition(osg::Vec3f(handX, handY, handZ));
        sph_trans->setPosition(osg::Vec3f(ballX, ballY, ballZ));
        box_trans->setPosition(osg::Vec3f(boxX, boxY, boxZ));

        if ((guiEventHandler->handState == false) && (closedHand == 1)) {
            guiEventHandler->handState = true;
        } else if ((guiEventHandler->handState == true) && (closedHand == 0)) {
            guiEventHandler->step = 0;
            guiEventHandler->handState = false;
        }



        guiEventHandler->iterate();


        ostringstream ostr;
        ostr<<"Time Left: "<<setprecision(0)<<fixed<<0/1000.0;
        textGeode->removeDrawables(0);
        textGeode->addDrawable( createText(
                                    osg::Vec3(50.0f, 50.0f, 0.0f),
                                    ostr.str(),
                                    35.0f)
                                );

        ostringstream score_ostr;
        score_ostr<<"Score: "<<score;
        scoreTextGeode->removeDrawables(0);
        scoreTextGeode->addDrawable( createText(
                                         osg::Vec3(50.0f, 1140.0f, 0.0f),
                                         score_ostr.str(),
                                         55.0f)
                                     );

        ostringstream target_ostr;
        target_ostr<<"Level: "<<level;
        targetTextGeode->removeDrawables(0);
        targetTextGeode->addDrawable( createText(
                                          osg::Vec3(50.0f, 1090.0f, 0.0f),
                                          target_ostr.str(),
                                          35.0f)
                                      );

        ostringstream subl_ostr;
        subl_ostr<<"Sub Level: "<<subLevel;
        sublTextGeode->removeDrawables(0);
        sublTextGeode->addDrawable( createText(
                                        osg::Vec3(50.0f, 1040.0f, 0.0f),
                                        subl_ostr.str(),
                                        35.0f)
                                    );
        ostringstream trial_ostr;
        trial_ostr<<"Trials Left: "<<trialPerSublevel - tmpTrial+1;
        TrialLeftGeode->removeDrawables(0);
        TrialLeftGeode->addDrawable( createText(
                                         osg::Vec3(50.0f, 990.0f, 0.0f),
                                         trial_ostr.str(),
                                         35.0f)
                                     );
    }


    for (size_t i=0; i<9; i++)
        fclose(eventFiles[i]);

    //int ret = 0;
    //ret = system("./copydata.sh");
    //cout<<"data copied"<<endl;

    //ret = system("matlab -nodesktop -nosplash -r \"plotdata;quit\"");
    //ret = system("reset");

    return visor.run();

}

