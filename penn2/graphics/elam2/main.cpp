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

    bool firstTrial = true;

    // root of scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group();

    // load arm model
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("../bare_hand_Scene.osgt");
    //cout<<int(!model)<<endl;

    // couldn't load the model
    if ( !model ) return 1;

    // add axes and arm to scene graph
    root->addChild(createAxis());

    osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform();
    pat->addChild(model);
    root->addChild(pat);

    osg::Vec3d handpos(0,0,0);

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

    visor.realize();
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
    camera->addChild( textGeode.get() );
    camera->addChild( scoreTextGeode.get() );
    camera->addChild( targetTextGeode.get() );
    camera->addChild( sublTextGeode.get() );
    camera->addChild( TrialLeftGeode.get() );
    camera->addChild( pauseTextGeode.get() );
    camera->getOrCreateStateSet()->setMode(
                GL_LIGHTING, osg::StateAttribute::OFF );

    HUD_Element * hud_element = new HUD_Element();
    hud_element->setElementPosition(50, 50);
    hud_element->setElementSize(1920-100, 1200-100);
    hud_element->setElementColor(visor.getCamera()->getClearColor()[0]-0.2, visor.getCamera()->getClearColor()[1]-0.2, visor.getCamera()->getClearColor()[2]-0.2, 0.9);

    HUD_Element * bioFeedBackground_1 = new HUD_Element();
    bioFeedBackground_1->setElementPosition(0, 0);
    bioFeedBackground_1->setElementSize(40, 120);
    bioFeedBackground_1->setElementColor(0.0, 0.0, 0.0, 1.0);

    HUD_Element * bioFeedBackground_1_frame = new HUD_Element();
    bioFeedBackground_1_frame->setElementPosition(0, 0);
    bioFeedBackground_1_frame->setElementSize(44, 124);
    //bioFeedBackground_1_frame->setElementColor(0.0, 1.0, 0.0, 1.0);
    bioFeedBackground_1_frame->setElementColor(30.0/255.0, 144.0/255.0, 255.0/255.0, 1.0);

    HUD_Element * bioFeedBackground_2 = new HUD_Element();
    bioFeedBackground_2->setElementPosition(0, 0);
    bioFeedBackground_2->setElementSize(40, 4);
    bioFeedBackground_2->setElementColor(30.0/255.0, 144.0/255.0, 255.0/255.0, 1.0);

    HUD_Element * bioFeedBackground_3 = new HUD_Element();
    bioFeedBackground_3->setElementPosition(0, 0);
    bioFeedBackground_3->setElementSize(40, 4);
    bioFeedBackground_3->setElementColor(30.0/255.0, 144.0/255.0, 255.0/255.0, 1.0);


    HUD_Element * bioFeedBox = new HUD_Element();
    bioFeedBox->setElementPosition(0, 0);
    bioFeedBox->setElementSize(20, 20);
    //bioFeedBox->setElementColor(0.996, 0.7333, 0.211, 1.0);
    bioFeedBox->setElementColor(200.0/255.0,20.0/255.0,200.0/255.0, 1.0);
    //bioFeedBox->setElementColor(1.0,1.0,1.0,1.0);
    bioFeedBox->HE_StateSet->setRenderBinDetails(12, "RenderBin");

    camera->addChild(bioFeedBackground_1_frame->getGeode());
    camera->addChild(bioFeedBackground_1->getGeode());
    camera->addChild(bioFeedBackground_2->getGeode());
    camera->addChild(bioFeedBackground_3->getGeode());
    camera->addChild(bioFeedBox->getGeode());

    HUD_Element_Circle * eyeHUD = new HUD_Element_Circle(0,0,40);
    /*HUD_Element * eyeHUD = new HUD_Element();
    eyeHUD->setElementPosition(0, 0);
    eyeHUD->setElementSize(50, 50);
    eyeHUD->setElementColor(200.0/255.0,20.0/255.0,200.0/255.0, 0.6);
    eyeHUD->HE_StateSet->setRenderBinDetails(15, "RenderBin");*/

    osg::ref_ptr<osg::PositionAttitudeTransform> eyeTrans = new osg::PositionAttitudeTransform();
    eyeTrans->setScale(osg::Vec3d(1.0,1.0,1.0));
    eyeTrans->setPosition(osg::Vec3d(0.0,0.0,0.0));
    eyeTrans->addChild(eyeHUD->getGeode());

    osg::Switch* switchEyeHUD = new osg::Switch();
    switchEyeHUD->addChild(eyeTrans, false);
    camera->addChild(switchEyeHUD);

    osg::Switch* switchHUD = new osg::Switch();

    summaryTextGeode.get()->getOrCreateStateSet()->setRenderBinDetails(11, "DepthSortedBin");
    timeLeftTextGeode.get()->getOrCreateStateSet()->setRenderBinDetails(12, "DepthSortedBin");
    //hud_element->getGeode()->getOrCreateStateSet()->setRenderBinDetails(0, "DepthSortedBin");

    switchHUD->addChild(hud_element->getGeode(),false);
    switchHUD->addChild(summaryTextGeode.get(), false);
    switchHUD->addChild(timeLeftTextGeode.get(), false);

    //t->getOrCreateStateSet()->setRenderBinDetails(3, "DepthSortedBin");

    camera->addChild(switchHUD);


    // for 3d
    // http://linux.die.net/man/3/popt
    //double fovy,aspectRatio,z1,z2;
    //visor.getCamera()->getProjectionMatrixAsPerspective(fovy,aspectRatio,z1,z2);
    //visor.getCamera()->setProjectionMatrixAsPerspective(fovy,0.75,z1,z2);

    //osg::ref_ptr<osg::NodeCallback> updatecallback = boneFinder._bones[38]->getUpdateCallback();
    //osgAnimation::UpdateBone* orig_upd = (osgAnimation::UpdateBone*) (boneFinder._bones[38]->getUpdateCallback());
    //osgAnimation::StackedTransform sfrm =  orig_upd->getStackedTransforms();

    //boneFinder._bones[38]->setInvBindMatrixInSkeletonSpace(boneFinder._bones[38]->getInvBindMatrixInSkeletonSpace() );

    //osg::Matrixd m = boneFinder._bones[38]->getMatrix();
    //cout<<m<<endl;

    //osg::Matrix rt = osg::Matrix::rotate(45.0*0.01745329, osg::Vec3(0.0f,1.0f,0.0f));
    //cout<<rt<<endl;
    //model->dirtyBound();


    // reading the initial oreintation and the positions of the bones
    {osgAnimation::UpdateBone* p = (osgAnimation::UpdateBone*)(boneFinder._bones[42]->getUpdateCallback());
        osgAnimation::StackedTransform st = p->getStackedTransforms();
        st.update();

        /*cout<<"mtx: "<<st.getMatrix()<<endl;
    cout<<"mtx_q: "<<st.getMatrix()(0,0)<<","<<st.getMatrix()(1,0)<<","<<st.getMatrix()(2,0)<<","<<st.getMatrix()(0,1)<<","<<st.getMatrix()(1,1)<<","<<st.getMatrix()(2,1)<<","<<st.getMatrix()(0,2)<<","<<st.getMatrix()(1,2)<<","<<st.getMatrix()(2,2)<<endl;
    cout<<"st: "<<st.size()<<endl;
    cout<<st.at(0)->getName()<<endl;
    cout<<st.at(1)->getName()<<endl;*/
        osg::ref_ptr<osgAnimation::StackedTransformElement> pp (st.at(1));
        osgAnimation::StackedTranslateElement* pt = static_cast<osgAnimation::StackedTranslateElement* > (pp.get());
        /*cout<<"tran: "<<pt->getTranslate()<<endl;
    cout<<st.at(2)->getName()<<endl;
    cout<<st.at(2)->getAsMatrix()<<endl;
    cout<<st.at(3)->getName()<<endl;*/
    }


    // transposed rotation submatrix
    Eigen::Matrix3d m;
    m << 0.0, -0.91015, -0.41429, -0.19892, 0.40601, -0.89196, 0.98002, 0.08241, -0.18104;
    //m << 1,0,0,0,1,0,0,0,1;
    Eigen::Quaterniond q1(m);
    Eigen::Matrix3d m2;
    m2 << -1, 0.00074, -0.00112, -0.00056, 0.52888, 0.8487, 0.00122, 0.8487, -0.52888;
    //m2 << 1,0,0,0,1,0,0,0,1;
    Eigen::Quaterniond q2(m2);
    Eigen::Matrix3d m3;
    m3 << -0.07944, -0.00064, -0.99684, -0.3643, 0.93085, 0.02843, 0.92789, 0.36541, -0.07418;
    //m3 << 1,0,0,0,1,0,0,0,1;
    Eigen::Quaterniond q3(m3);
    Eigen::Matrix3d m4;
    m4 << -0.02652,0.06174,0.99774,0.22916,0.97189,-0.05405,-0.97303,0.22721,-0.03992;
    //m4 << 1,0,0,0,1,0,0,0,1;
    Eigen::Quaterniond q4(m4);
    Eigen::Matrix3d m5;
    m5 << -0.36038,0,-0.9328,0,1,0,0.9328,0,-0.36038;
    Eigen::Quaterniond q5(m5);

    Eigen::Vector3d joint1_rot(-0.921228,-0.0670103,0.383208);
    Eigen::Vector3d joint2_rot(0.410745,0.909544,-0.0633911);
    Eigen::Vector3d joint3_rot(0.791999,-0.186944,0.581197);

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



    ////////////////////////// DEBUG

    osg::Sphere* sphere_d1 = new osg::Sphere( osg::Vec3(0.0,0.0,0.0), 0.3);
    osg::ShapeDrawable* sphereDrawable_d1 = new osg::ShapeDrawable(sphere_d1);
    sphereDrawable_d1->setColor(osg::Vec4(1.0, 0.8, 0.8, 1.0));
    sphereDrawable_d1->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    osg::Geode* sphereGeode_d1 = new osg::Geode();
    osg::ref_ptr<osg::PositionAttitudeTransform> sph_trans_d1 = new osg::PositionAttitudeTransform();
    sph_trans_d1->setScale(osg::Vec3d(1.0,1.0,1.0));
    //osg::Vec3d sph_pos(-3.0, -5.0, 4.0);
    osg::Vec3d sph_pos_d1(-11, -1.6388, 0.75597);
    sph_trans_d1->setPosition(sph_pos_d1);
    sph_trans_d1->setAttitude(osg::Quat(0, osg::Vec3d(0,0,0)));
    sph_trans_d1->addChild(sphereGeode_d1);
    //root->addChild(sph_trans_d1);
    sphereGeode_d1->addDrawable(sphereDrawable_d1);


    /*********************************************************************************
     *
     *
     *This is the code of thetraffic light
     *
     *
     *********************************************************************/
    osg::Sphere* unitSphere = new osg::Sphere( osg::Vec3(0,0,0), 1.0);
    osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(unitSphere);
    unitSphereDrawable->setColor(osg::Vec4(1.0, 0.8, 0.8, 1.0));
    unitSphereDrawable->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    osg::Geode* sphereGeode_unit = new osg::Geode();
    osg::ref_ptr<osg::PositionAttitudeTransform> sph_trans_unit = new osg::PositionAttitudeTransform();
    sph_trans_unit->setScale(osg::Vec3d(1.0,1.0,1.0));
    //osg::Vec3d sph_pos(-3.0, -5.0, 4.0);
    osg::Vec3d sph_pos_unit(-10, -1.0, 5);
    sph_trans_unit->setPosition(sph_pos_unit);
    sph_trans_unit->setAttitude(osg::Quat(0, osg::Vec3d(0,0,0)));
    sph_trans_unit->addChild(sphereGeode_unit);
    root->addChild(sph_trans_unit);
    //sphereGeode_unit->addDrawable(unitSphereDrawable);



    /////////////////////////////////////////////////////////////////


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


    float elbow_angle = 0.0;

    bool stickBallHand = true;

    auto start = high_resolution_clock::now();
    auto start_waypoints = high_resolution_clock::now();
    auto start_enter_waypoints = high_resolution_clock::now();
    auto start_final = high_resolution_clock::now();



    int inReach = 0;
    int inBoxReach = 0;

    int score = 0;

    vector<float> shoulderangles(3);
    vector<float> elbowangles(3);
    vector<float> shoulder2angles(3);
    double fixed1 = 0.0;
    double fixed2 = 0.0;
    double fixed3 = 0.0;

    /**********************************************************************
     *This part sets the sockets to connect with the modules
     *********************************************************************/
    context_t context(1);
    socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("ipc:///tmp/graphics.pipe");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    //socket_t subscriber_eeg(context, ZMQ_SUB);
    //subscriber_eeg.connect("tcp://127.0.0.1:5559");
    //cout <<"passed the first one"<<endl;
    //socket_t publisher_eeg(context, ZMQ_PUB);
    //publisher_eeg.bind("tcp://127.0.0.1:55000");
    //cout <<"passed the first one"<<endl;
    //socket_t publisher_histogram(context, ZMQ_PUB);
    //publisher_histogram.bind("tcp://127.0.0.1:55003");
    //socket_t publisher_hand(context, ZMQ_PUB);
    //publisher_hand.bind("tcp://127.0.0.1:55008");

    //cout <<"passed the first one"<<endl;
    //socket_t publisher(context, ZMQ_PUB);
    //publisher.bind("ipc:///tmp/game.pipe");

    //socket_t eyeSubscriber(context, ZMQ_SUB);
    //eyeSubscriber.connect("ipc:///tmp/eye.pipe");
    //eyeSubscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    /**************************************************************************/

    zmq::message_t hand_msg;
    zmq::message_t eye_msg;

    size_t level = 1;
    size_t subLevel = 1;
    size_t tmpTrial = 1;
    size_t paused = 0;
    int eegclick[3] = {0}; //eegclick holds 2 pieces of information [0] holds the release state, [1] holds the moment when the subject has top start trying
    int lastFrameEEGClick = 0;
    float message_arr[1]; //Holds the message that will be sent to the histogram
    int summary_count = 0;


    osgViewer::Viewer::Windows sWindows;
    visor.getWindows(sWindows);
    sWindows[0]->setCursor(osgViewer::GraphicsWindow::NoCursor);
    auto gameStartTime = high_resolution_clock::now();
    auto counter_start = high_resolution_clock::now();
    const float alpha = 0.2;
    bool firstIt = true;

    int new_duration = 90000;
    int base_duration = 90000;
    int centerBox = -50; // between -50 and +50
    auto trial_counter_start = high_resolution_clock::now(); //this counter resets each time it passes in the last point
    auto droput_clock = high_resolution_clock::now();//this counter checks each time there is a drop

    bool targetEngaged = false;
    bool waypointEngaged = false;


    float trialNumber = 0.0;

    // first trial event
    {
        std::chrono::milliseconds dura( 500 );
        this_thread::sleep_for(dura);
        auto tmpTrialTime = high_resolution_clock::now();
        milliseconds tmpMS = duration_cast<milliseconds>(tmpTrialTime - gameStartTime);
        float tmpTrialTimeFlt = float(tmpMS.count()/1000.0);

        fwrite(&tmpTrialTimeFlt, sizeof(float), 1, eventFiles[0]);
        fwrite(&trialNumber, sizeof(float), 1, eventFiles[0]);
        fflush(eventFiles[0]);
        cout<<"wrote into file "<<tmpTrialTimeFlt<<endl;
    }


    const float boxPickRadius = 1.2;

    // event: ball picked
    {
        auto tmpBallTime = high_resolution_clock::now();
        milliseconds tmpMS = duration_cast<milliseconds>(tmpBallTime - gameStartTime);
        float tmpBallTimeFlt = float(tmpMS.count()/1000.0);

        fwrite(&tmpBallTimeFlt, sizeof(float), 1, eventFiles[4]);

        float bx = sph_trans->getPosition().x() / (boxPickRadius * 2.0);
        float by = sph_trans->getPosition().y() / (boxPickRadius * 2.0);
        float bz = sph_trans->getPosition().z() / (boxPickRadius * 2.0);
        fwrite(&bx, sizeof(float), 1, eventFiles[4]);
        fwrite(&by, sizeof(float), 1, eventFiles[4]);
        fwrite(&bz, sizeof(float), 1, eventFiles[4]);
        fwrite(&trialNumber, sizeof(float), 1, eventFiles[4]);

        fflush(eventFiles[4]);
    }

    bool wasHoldingBall = false;
    float totalTimeWithoutBall = 0.0;
    float totalTimeWithBall = 0.0;
    float totalTimeTargetEngaged = 0.0;
    bool startTargetEngagedTimer = false;
    auto targetEngagedTimer = high_resolution_clock::now();
    auto withBallTDTimer = high_resolution_clock::now();
    auto withoutBallTDTimer = high_resolution_clock::now();
    auto pickup_clock = high_resolution_clock::now();//this counter checks each time there is a pickup


    float totalNumberOfDrops = 0;

    float timeTargetEngaged = 0.0;

    float lastWaypointEngaged = 0.0;

    int framesStareTarget = 0;
    int framesNotStareTarget = 0;


    while ( !visor.done()){

        osg::Matrix MVPW(visor.getCamera()->getViewMatrix() *
                         visor.getCamera()->getProjectionMatrix() *
                         visor.getCamera()->getViewport()->computeWindowMatrix());
        osg::Vec3 posIn2D = pat->getPosition() * MVPW;

        osg::Vec3 targetPosIn2D = box_trans->getPosition() * MVPW;

        bioFeedBox->setElementPosition(posIn2D.x()+190, posIn2D.y()-120+centerBox);
        bioFeedBackground_1_frame->setElementPosition(posIn2D.x()+178, posIn2D.y()-162);
        bioFeedBackground_1->setElementPosition(posIn2D.x()+180, posIn2D.y()-160);
        bioFeedBackground_2->setElementPosition(posIn2D.x()+180, posIn2D.y()-110);
        bioFeedBackground_3->setElementPosition(posIn2D.x()+180, posIn2D.y()-135);

        //jus
        auto end = high_resolution_clock::now();
        auto target = high_resolution_clock::duration(new_duration);
        milliseconds final_time = duration_cast<milliseconds>(target);
        milliseconds ms = duration_cast<milliseconds>(end - start);
        milliseconds ms_wayp = duration_cast<milliseconds>(end - start_waypoints);
        milliseconds ms_ent_wayp = duration_cast<milliseconds>(end - start_enter_waypoints);
        milliseconds ms_final = duration_cast<milliseconds>(end - start_final);
        milliseconds ms_dropout = duration_cast<milliseconds>(end - droput_clock);
        milliseconds ms_pickup = duration_cast<milliseconds>(end- pickup_clock);

        //cout<<"Click is"<<eegclick[0]<<endl;
        //zmq::message_t eeg_signal;
        //bool gotSomthing = subscriber_eeg.recv(&eeg_signal,ZMQ_NOBLOCK);
        //cout<<"gotSomthing "<<int(gotSomthing)<<endl;
        //cout<<"Received is "<<eeg_signal.data()<<endl;
        //if (gotSomthing)
        //memcpy(eegclick, eeg_signal.data(), 3*sizeof(int));

        if ((lastFrameEEGClick == 1) && (eegclick[0] == 1)) {
            eegclick[0] = 0;
        }

        if (eegclick[0] == 0) {
            lastFrameEEGClick = 0;
        }

        if ((eegclick[0] == 1) && (lastFrameEEGClick == 0)) {
            lastFrameEEGClick = 1;
        }

        paused = (size_t) pauseGame;
        //cout<<"Click is "<<eegclick[0]<<endl;
        //cout<<"Power is "<<eegclick[2]<<endl;
        //cout<<"Mean Power is"<<float(eegclick[2])/10000<<endl;
        centerBox = PowerToPixel(float(eegclick[2])/10000);
        //cout <<"Box is "<<centerBox<<endl;
        //cout<<"The Waypoint is "<<currentWaypoint<<endl;
        //if (demoMode)
        //    eegclick[0] = 0;
        int waypoint_message[7];
        waypoint_message[0] = currentWaypoint;
        waypoint_message[1] = final_time.count()-ms.count()/1000;
        waypoint_message[2] = ms_wayp.count();
        waypoint_message[3] = ms_ent_wayp.count();
        waypoint_message[4] = ms_final.count();
        waypoint_message[5] = ms_dropout.count();
        waypoint_message[6] = ms_pickup.count();

        //cout<<"The Sent Waypoint is "<<waypoint_message[0]<<endl;
        //zmq::message_t waypoint_zmq(7*sizeof(int));
        //memcpy(waypoint_zmq.data(), &(waypoint_message[0]), 7*sizeof(int));
        //publisher_eeg.send(waypoint_zmq);
        /******************************************************************
         *Management of the traffic light
         *********************************************************************/

        //if (currentWaypoint>=0)
        //{
        //    sphereDrawable->setColor(osg::Vec4(238.0/255.0, 0, 0, 1.0));//red

        //}
        if (eegclick[1]==0)
        {
            sphereDrawable->setColor(osg::Vec4(255.0/255.0, 165.0/255.0, 0, 1.0));//amber

        }
        if (eegclick[1]==1)
        {
            sphereDrawable->setColor(osg::Vec4(0, 1.0, 0, 1.0));//green

        }





        // send game state over zmq
        // paused/playing: size_t
        // trial num: size_t
        // sublevel num: size_t
        // level num: size_t
        // score: int
        // size_t is an unint


        //zmq::message_t zmq_message(4*sizeof(char) + sizeof(int));
        //memcpy(zmq_message.data(), &paused, sizeof(size_t));
        //memcpy((size_t *)zmq_message.data()+1, &tmpTrial, sizeof(size_t));
        //memcpy((size_t *)zmq_message.data()+2, &subLevel, sizeof(size_t));
        //memcpy((size_t *)zmq_message.data()+3, &level, sizeof(size_t));
        //memcpy((size_t *)zmq_message.data()+4, &score, sizeof(int));
        //publisher.send(zmq_message);

        //This ocntrols the pause button
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
        //sublevel flag raises when a level has been finished
        //If the flag is raised, we show the summary score for 10 seconds
        /*
        if (subLevelFlag){
            switchHUD->setValue(0, true);
            switchHUD->setValue(1, true);
            switchHUD->setValue(2, true);
            pauseTextGeode->removeDrawables(0);
            pauseTextGeode->addDrawable( createText(
              osg::Vec3(350.0f, 1040.0f, 0.0f),
                                           "Summary Screen",
              95.0f, false)
            );
            ostringstream summary_ostr;
            summary_ostr<<"Score for this Level: "<<score;
            summaryTextGeode->removeDrawables(0);
            summaryTextGeode->addDrawable( createText(
              osg::Vec3(500.0f, 500.0f, 0.0f),
              summary_ostr.str(),
              35.0f)
            );
            ostringstream timeleft_ostr;
            timeleft_ostr<<"Time Left: "<<setprecision(0)<<fixed<<final_time.count()-ms.count()/1000.0;
            timeLeftTextGeode->removeDrawables(0);
            timeLeftTextGeode->addDrawable( createText(
              osg::Vec3(500.0f, 460.0f, 0.0f),
              timeleft_ostr.str(),
              35.0f)
            );
            summary_count++;
            subscriber.recv(&hand_msg);//We receive the message, otherwise it starts buffering
            currentWaypoint = -2;
            if (summary_count>=250){
                subLevelFlag = false;
                summary_count =0;
                currentWaypoint = 0;
            }

            continue;
        } else {
            pauseTextGeode->removeDrawables(0);
            switchHUD->setValue(0, false);
            switchHUD->setValue(1, false);
            switchHUD->setValue(2, false);
        }*/



        //cout<<"*******************"<<endl;
        //cout<<visor.getCameraManipulator()->getMatrix()<<endl;
        //cout<<"-------------------"<<endl;

        /*int eyeX = -1000;
        int eyeY = -1000;
        bool rEye = eyeSubscriber.recv(&eye_msg, ZMQ_NOBLOCK);
        string rpl;
        while (rEye) {
            rEye = eyeSubscriber.recv(&eye_msg, ZMQ_NOBLOCK);
            if (rEye) {
                rpl = string(static_cast<char*>(eye_msg.data()), eye_msg.size());
                cout<<"EYE: "<<rpl<<endl;

                istringstream iss(rpl);
                vector<string> eyePos;
                copy(istream_iterator<string>(iss),
                     istream_iterator<string>(),
                     back_inserter<vector<string> >(eyePos));
                istringstream(eyePos[0]) >> eyeX;
                istringstream(eyePos[1]) >> eyeY;
                //cout<<"ex: "<<eyeX<<"  ey: "<<eyeY<<endl;
            }
        }*/


        // update next frame

        switchEyeHUD->setValue(0, false);
        visor.frame();
        /*if (eyeMode && (eyeX != -1000)){
            eyeTrans->setPosition(osg::Vec3(eyeX,1080-eyeY,0));
            switchEyeHUD->setValue(0, true);
        }*/
        if (showDuplicateWindow)
            visor2.frame();


        subscriber.recv(&hand_msg);

        string hand_str(((char *)hand_msg.data()));
        cout<<"hand_str "<<hand_str<<endl;
        //stringstream ss;
        //ss.str(hand_str);

        float tx,ty,tz;
        //ss>>tx>>tz>>ty;
        //cout<<"---------------"<<endl;
        //cout<<tx<<endl;
        //cout<<ty<<endl;
        //cout<<tz<<endl;
        tx = (tx - 320.0) / 26.0 + 2.0;
        tz = -(tz - 240.0) / 26.0 + 2.0;
        ty = -(ty - 32.0) * 3.0;
        //cout<<tx<<endl;
        //cout<<ty<<endl;
        //cout<<tz<<endl;

        // here we pubslish the hand values consistent with what the user is looking at
        //float hand_message[3]; //hand message has x, y and z position
        //hand_message[0] = tx;
        //hand_message[1] = ty;
        //hand_message[2] = tz;
        //zmq::message_t hand_signal(3*sizeof(float));
        //memcpy(hand_signal.data(), &(hand_message[0]), 3*sizeof(float));
        //publisher_hand.send(hand_signal);

        if (!firstIt) {
//            handpos.x() = handpos.x() + alpha * (tx - handpos.x());
//            handpos.z() = handpos.z() + alpha * (tz - handpos.z());
//            handpos.y() = handpos.y() + alpha * (ty - handpos.y());
        } else {
//            handpos.x() = tx;
//            handpos.z() = tz;
//            handpos.y() = ty;
        }
        firstIt = false;


        if (dimension < 3)
            handpos.y() = 4.0;

        if (dimension < 2)
            handpos.z() = 4.0;


        /*cout<<"handpos"<<endl;
        cout<<handpos.x()<<endl;
        cout<<handpos.y()<<endl;
        cout<<handpos.z()<<endl;*/
        //cout<<"-----------"<<endl;

        pat->setPosition(handpos);

        guiEventHandler->iterate();


        //double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 10.0;
        ostringstream ostr;
        ostr<<"Time Left: "<<setprecision(0)<<fixed<<final_time.count()-ms.count()/1000.0;
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


        float diff_roll = (roll2-roll);
        if (fabs(diff_roll)>10.0){
            float tmp_elbow_angle = elbow_angle;
            tmp_elbow_angle += (diff_roll*0.01);
            if ((tmp_elbow_angle > -20.0) && (tmp_elbow_angle < 60.0))
                elbow_angle = tmp_elbow_angle;
        }

        const osg::MatrixList& m = boneFinder._bones[51]->getWorldMatrices();
        osg::ComputeBoundsVisitor cbv;
        boneFinder._bones[51]->accept( cbv );
        osg::Vec3 centB = osg::Vec3(0.0, 0.0, 0.0) * m.front();

        // IK
        osg::Vec3 shoulderPos = osg::Vec3(0.0, 0.0, 0.0) * boneFinder._bones[38]->getWorldMatrices().front();
        //cout<<"sp: "<<shoulderPos<<endl;

        osg::Quat tmp_quat;
        //tmp_quat.makeRotate(fixed1, osg::Vec3(joint1_rot(0),joint1_rot(1),joint1_rot(2)));
        osg::Quat tmp_quat2;
        //tmp_quat2.makeRotate(fixed2, osg::Vec3(joint2_rot(0),joint2_rot(1),joint2_rot(2)));

        Eigen::Quaterniond eq_joint1(tmp_quat.w(), tmp_quat.x(), tmp_quat.y(), tmp_quat.z());
        Eigen::Quaterniond eq_joint2(tmp_quat2.w(), tmp_quat2.x(), tmp_quat2.y(), tmp_quat2.z());

        Eigen::Vector3d current_joint2_rot =  eq_joint1.inverse() * q1.inverse() *  joint2_rot;
        Eigen::Vector3d current_joint3_rot =  q4.inverse()  *  q3.inverse()  * eq_joint2.inverse() * q2.inverse()  * eq_joint1.inverse() * q1.inverse() *  joint2_rot;
        //Eigen::Vector3d current_joint3_rot =  q4.inverse()  *  q3.inverse()  * q2.inverse()  * eq_yaw.inverse() * q1.inverse() *  joint1_rot;


        double joint1_ik_rot = 0;
        double joint2_ik_rot = 0;
        double joint3_ik_rot = 0;
        //solveIK(sph_trans->getPosition(), centB, shoulderPos, joint1_ik_rot, joint1_rot);

        //cout<<"shoulder correction: "<<shoulderangles[2]<<endl;
        osg::Quat q_yaw;
        //fixed1 += joint1_ik_rot;
        fixed1 = -0.7;
        //q_yaw.makeRotate(fixed1, osg::Vec3(0.0f,1.0f,0.0f));

        //q_yaw = q_yaw_o;
        //cout<<"yaw: "<<q_yaw<<endl;

        osg::Vec3 shoulder2Pos = osg::Vec3(0.0, 0.0, 0.0) * boneFinder._bones[39]->getWorldMatrices().front();
        //solveIK(sph_trans->getPosition(), centB, shoulder2Pos, joint2_ik_rot, current_joint2_rot);

        osg::Quat q_pitch;
        //fixed2 += (shoulderangles[0]);
        //fixed2 += joint2_ik_rot;
        fixed2 = -3.141592/2.0 + 0.5;
        q_pitch.makeRotate(fixed2, osg::Vec3(1.0f,0.0f,0.0f));
        osg::Quat q_roll;
        //q_roll.makeRotate(((elbow_angle)*2.0+30.0)*0.01745329, osg::Vec3(0.0f,0.0f,1.0f));


        osg::Vec3 elbowPos = osg::Vec3(0.0, 0.0, 0.0) * boneFinder._bones[41]->getWorldMatrices().front();
        //solveIK(sph_trans->getPosition(), centB, elbowPos, joint3_ik_rot, current_joint3_rot, 0.019);
        //sph_trans_d1->setPosition(elbowPos);

        //fixed3 += joint3_ik_rot;
        fixed3 = 3.141592/2.0;

        osg::Quat f_yaw;
        f_yaw.makeRotate(fixed3, osg::Vec3(0.0f,0.0f,1.0f));

        const osg::MatrixList& mw = boneFinder._bones[42]->getWorldMatrices();

        Eigen::Matrix3d rot;
        rot<<mw.front()(0,0),mw.front()(0,1),mw.front()(0,2),mw.front()(1,0),mw.front()(1,1),mw.front()(1,2),mw.front()(2,0),mw.front()(2,1),mw.front()(2,2);

        Eigen::Vector3d ea = rot.eulerAngles(2, 0, 2);

        osg::Quat wrist_roll;
        //wrist_roll.makeRotate(-(0), osg::Vec3(1.0f,0.0f,0.0f));


        // update bones
        osgAnimation::UpdateBone* shoulder_update = new osgAnimation::UpdateBone("shoulder.R");
        shoulder_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate1", osg::Vec3(-0.05423, 0.16051, -0.04292)));
        shoulder_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate1", q_yaw*osg::Quat(q1.x(), q1.y(), q1.z(), q1.w())));
        boneFinder._bones[38]->setUpdateCallback(shoulder_update);

        osgAnimation::UpdateBone* upper_arm_update = new osgAnimation::UpdateBone("upper_arm.R");
        upper_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate2", osg::Vec3(0.0, 0.13775, 0.000)));
        upper_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate2", q_pitch*osg::Quat(q2.x(), q2.y(), q2.z(), q2.w())));
        boneFinder._bones[39]->setUpdateCallback(upper_arm_update);

        osgAnimation::UpdateBone* upper_arm2_update = new osgAnimation::UpdateBone("upper_arm.R001");
        upper_arm2_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate3", osg::Vec3(0.0, 0.15716, 0.0)));
        //upper_arm2_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate3", q_roll*osg::Quat(q3.x(), q3.y(), q3.z(), q3.w())));
        upper_arm2_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate3", osg::Quat(q3.x(), q3.y(), q3.z(), q3.w())));
        boneFinder._bones[40]->setUpdateCallback(upper_arm2_update);

        osgAnimation::UpdateBone* forearm_arm_update = new osgAnimation::UpdateBone("forearm_arm.R");
        forearm_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate4", osg::Vec3(0.00, 0.12632, 0.0)));
        //forearm_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate4", q_yaw2*osg::Quat(q4.x(), q4.y(), q4.z(), q4.w())));
        forearm_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate4", f_yaw*osg::Quat(q4.x(), q4.y(), q4.z(), q4.w())));
        boneFinder._bones[41]->setUpdateCallback(forearm_arm_update);

        osgAnimation::UpdateBone* forearm_arm2_update = new osgAnimation::UpdateBone("forearm_arm.R001");
        forearm_arm2_update->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate5", osg::Vec3(0.00, 0.13282, 0.0)));
        //forearm_arm_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate5", q_yaw2*osg::Quat(q4.x(), q4.y(), q4.z(), q4.w())));
        forearm_arm2_update->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("rotate5", wrist_roll*osg::Quat(q5.x(), q5.y(), q5.z(), q5.w())));
        boneFinder._bones[42]->setUpdateCallback(forearm_arm2_update);

        // beep if palm hits the cylinder
        osgAnimation::UpdateBone* p = (osgAnimation::UpdateBone*)(boneFinder._bones[44]->getUpdateCallback());
        osgAnimation::StackedTransform st = p->getStackedTransforms();
        boneFinder._bones[44]->dirtyBound();

        //osgAnimation::Bone b;

        st.update();
        osg::BoundingBox bb = cbv.getBoundingBox();
        osg::Vec3 centV = bb.center() * m.front();
        //cout<<boneFinder._bones[44]->computeBound().center()<<endl;

        //cout<<centV<<" - "<< sph_trans->getBound().center()<<endl;


        //**********************************************************************************************************
        //cout<<"hand: "<<centB<<endl;
        //cout<<"ball:"<<sph_trans->getPosition()<<endl;


        osg::Matrixd mm = m.front();
        osg::Matrixd imm = osg::Matrixd::inverse(mm);

        // three unit vecs, get their coordinates in the world space
        osg::Vec3 iVec = osg::Vec3(1.0, 0.0, 0.0) * mm;
        osg::Vec3 jVec = osg::Vec3(0.0, 1.0, 0.0) * mm;
        osg::Vec3 kVec = osg::Vec3(0.0, 0.0, 1.0) * mm;
        osg::Vec3 oVec = osg::Vec3(0.0, 0.0, 0.0) * mm;

        osg::Vec3 localBallVec = sph_trans->getPosition() - oVec;
        osg::Vec3 localBoxVec = box_trans->getPosition() - oVec;

        //cout<<"b: "<<localBallVec<<endl;

        //osg::Matrix3 cvtCoord(iVec.x(), iVec.y(), iVec.z(),  jVec.x(), jVec.y(), jVec.z(),  kVec.x(), kVec.y(), kVec.z());
        //osg::Vec3 localBallCoord = cvtCoord * localBallVec;

        // normalize the subtracted Vec from oVec
        iVec = iVec - oVec;
        jVec = jVec - oVec;
        kVec = kVec - oVec;
        //cout<<"k: "<<kVec<<endl;
        iVec.normalize();
        jVec.normalize();
        kVec.normalize();

        // *************************
        // modular signal simulator, several different frequencies
        // ask Leon to implement the posterior probability from the whole eeg whether subject is paying attention
        // and also the same code for extracting power from frequency band
        // **********************************


        Eigen::Matrix3d cvtCoord;
        cvtCoord<<iVec.x(), iVec.y(), iVec.z(),  jVec.x(), jVec.y(), jVec.z(),  kVec.x(), kVec.y(), kVec.z();
        Eigen::Vector3d localEiBallVec(localBallVec.x(), localBallVec.y(), localBallVec.z());
        Eigen::Vector3d localEiBoxVec(localBoxVec.x(), localBoxVec.y(), localBoxVec.z());

        Eigen::Vector3d MatrlocalBallCoord = cvtCoord * localEiBallVec;
        Eigen::Vector3d MatrlocalBoxCoord = cvtCoord * localEiBoxVec;
        Eigen::Vector3d MatrTargetBoxCoord = cvtCoord * localEiBoxVec;
        //cout<<"v: "<<MatrlocalBallCoord<<endl;
        //cout<<"dis: "<<MatrlocalBallCoord.norm()<<endl;

        //cout<<iVec<<endl;


        double distanceToWaypoint = 0.0;

        if (currentWaypoint != -1) {
            osg::Vec3 localBoxVec = wPoints[currentWaypoint]->getPosition() - oVec;
            Eigen::Vector3d localEiBoxVec(localBoxVec.x(), localBoxVec.y(), localBoxVec.z());
            Eigen::Vector3d MatrlocalBoxCoord = cvtCoord * localEiBoxVec;
            distanceToWaypoint = MatrlocalBoxCoord.norm();
        }

        // DEBUG - TO BE REMOVED
        /*{osgAnimation::UpdateBone* p = (osgAnimation::UpdateBone*)(boneFinder._bones[42]->getUpdateCallback());
            osgAnimation::StackedTransform st = p->getStackedTransforms();
            st.update();

            cout<<"mtx: "<<st.getMatrix()<<endl;
            //Eigen::Quaternion qt;

            const osg::MatrixList& m = boneFinder._bones[42]->getWorldMatrices();
            cout<<"w: "<<m.front()<<endl;

            Eigen::Matrix3d rot;
            rot<<m.front()(0,0),m.front()(0,1),m.front()(0,2),m.front()(1,0),m.front()(1,1),m.front()(1,2),m.front()(2,0),m.front()(2,1),m.front()(2,2);
            cout<<"m: "<<rot<<endl;
            Eigen::Vector3d ea = rot.eulerAngles(2, 0, 2);
            cout<<"e: "<<ea<<endl;

            //cout<<"mtx_q: "<<st.getMatrix()(0,0)<<","<<st.getMatrix()(1,0)<<","<<st.getMatrix()(2,0)<<","<<st.getMatrix()(0,1)<<","<<st.getMatrix()(1,1)<<","<<st.getMatrix()(2,1)<<","<<st.getMatrix()(0,2)<<","<<st.getMatrix()(1,2)<<","<<st.getMatrix()(2,2)<<endl;
            cout<<"st: "<<st.size()<<endl;
            cout<<st.at(0)->getName()<<endl;
            cout<<st.at(1)->getName()<<endl;
            osg::ref_ptr<osgAnimation::StackedTransformElement> pp (st.at(1));
            osgAnimation::StackedTranslateElement* pt = static_cast<osgAnimation::StackedTranslateElement* > (pp.get());
            //cout<<"tran: "<<pt->getTranslate()<<endl;
            //cout<<st.at(2)->getName()<<endl;
            //cout<<st.at(2)->getAsMatrix()<<endl;
            //cout<<st.at(3)->getName()<<endl;
        }*/

        // 1. get the new coordinate frame for the hand
        // 2. bounding box of the grasp
        // 3. check if the ball is in the grasp box

        // if the hand hits the sphere, stick the sphere to the hand
        /*if (centV.x() > (sph_pos.x()-1.5) &&
                centV.x() < (sph_pos.x()+1.5) &&
                centV.y() > (sph_pos.y()-1.5) &&
                centV.y() < (sph_pos.y()+1.5) &&
                centV.z() > (sph_pos.z()-1.5) &&
                centV.z() < (sph_pos.z()+1.5)
                ) {
            // move the sphere to a new random position
            //sph_pos.x() = (double)rand()/(RAND_MAX)*(-4.0);
            //sph_pos.y() = (double)rand()/(RAND_MAX)*(-2.0)-4.0;
            //sph_pos.z() = (double)rand()/(RAND_MAX)*(-4.0)+2.0;
            //sph_trans->setPosition(sph_pos);
            //sph_trans->dirtyBound();

            stickBallHand = true;

        }*/

        sph_trans->setPosition(sph_pos+osg::Vec3(ball_x, ball_y, ball_z));
        //sph_trans_unit->setPosition(sph_pos+osg::Vec3(ball_x, ball_y, ball_z));



        // grasp the ball
        if (MatrlocalBallCoord.norm() < 1.1){ //used to be 1.1
            inReach +=1;


            if ((inReach > 40)&&(guiEventHandler->handState == false)) {
                inReach = 0;
                guiEventHandler->step = 0;
                guiEventHandler->handState = true;
                stickBallHand = true;
                pickup_clock = high_resolution_clock::now();


                // event: ball picked
                {
                    auto tmpBallTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpBallTime - gameStartTime);
                    float tmpBallTimeFlt = float(tmpMS.count()/1000.0);

                    fwrite(&tmpBallTimeFlt, sizeof(float), 1, eventFiles[4]);

                    float bx = sph_trans->getPosition().x() / (boxPickRadius * 2.0);
                    float by = sph_trans->getPosition().y() / (boxPickRadius * 2.0);
                    float bz = sph_trans->getPosition().z() / (boxPickRadius * 2.0);
                    fwrite(&bx, sizeof(float), 1, eventFiles[4]);
                    fwrite(&by, sizeof(float), 1, eventFiles[4]);
                    fwrite(&bz, sizeof(float), 1, eventFiles[4]);
                    fwrite(&trialNumber, sizeof(float), 1, eventFiles[4]);
                    fflush(eventFiles[4]);
                }


                /*sph_pos.x() = (double)rand()/(RAND_MAX)*(-5.0);
                sph_pos.y() = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
                sph_pos.z() = (double)rand()/(RAND_MAX)*(-4.0)+2.0;
                sph_trans->setPosition(sph_pos);*/


                //thread dingth(dingMusic);
                //dingth.join();
            }
        } else {
            inReach = 0;
        }

        // waypoint


        //cout<<tmpTrial<<" of "<<trialpersubl.getValue()<<endl;
        //cout<<subLevel<<" of "<<sublperlevel.getValue()<<" level"<<endl;

        if (currentWaypoint != -1) {


            //cout<<"wp"<<currentWaypoint<<endl;
            wPoints[currentWaypoint]->setScale(osg::Vec3d(1.0,1.0,1.0));
            if (distanceToWaypoint < boxPickRadius) {
                inBoxReach +=1;

                // event: waypoint engaged
                if (!waypointEngaged) {

                    auto tmpTargetTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpTargetTime - gameStartTime);
                    float tmpTargetTimeFlt = float(tmpMS.count()/1000.0);

                    fwrite(&tmpTargetTimeFlt, sizeof(float), 1, eventFiles[7]);

                    float cwp = currentWaypoint+1;
                    lastWaypointEngaged = cwp;
                    fwrite(&cwp, sizeof(float), 1, eventFiles[7]);
                    float wpx = wPoints[currentWaypoint]->getPosition().x() / (boxPickRadius * 2.0);
                    float wpy = wPoints[currentWaypoint]->getPosition().y() / (boxPickRadius * 2.0);
                    float wpz = wPoints[currentWaypoint]->getPosition().z() / (boxPickRadius * 2.0);
                    fwrite(&wpx, sizeof(float), 1, eventFiles[7]);
                    fwrite(&wpy, sizeof(float), 1, eventFiles[7]);
                    fwrite(&wpz, sizeof(float), 1, eventFiles[7]);
                    fwrite(&trialNumber, sizeof(float), 1, eventFiles[7]);
                    fflush(eventFiles[7]);

                    waypointEngaged = true;

                }
                start_enter_waypoints = high_resolution_clock::now();
                //cout<<"wp "<<inBoxReach<<endl;
                if (inBoxReach > 20) {
                    inBoxReach = 0;
                    wPoints[currentWaypoint]->setScale(osg::Vec3d(0.8,0.8,0.8));
                    currentWaypoint += 1;
                    start_waypoints = high_resolution_clock::now();

                    if (currentWaypoint >= numWaypoints) {
                        //start the counter
                        box_trans->setScale(osg::Vec3d(1.3,1.3,1.3));
                        currentWaypoint = -1;
                        score += 1;
                    }
                }

            } else {
                inBoxReach = 0;

                if (waypointEngaged) {
                    waypointEngaged = false;

                    {
                        auto tmpTargetTime = high_resolution_clock::now();
                        milliseconds tmpMS = duration_cast<milliseconds>(tmpTargetTime - gameStartTime);
                        float tmpTargetTimeFlt = float(tmpMS.count()/1000.0);

                        fwrite(&tmpTargetTimeFlt, sizeof(float), 1, eventFiles[8]);

                        fwrite(&lastWaypointEngaged, sizeof(float), 1, eventFiles[8]);
                        fwrite(&trialNumber, sizeof(float), 1, eventFiles[8]);

                        float wpx = wPoints[int(lastWaypointEngaged)-1]->getPosition().x() / (boxPickRadius * 2.0);
                        float wpy = wPoints[int(lastWaypointEngaged)-1]->getPosition().y() / (boxPickRadius * 2.0);
                        float wpz = wPoints[int(lastWaypointEngaged)-1]->getPosition().z() / (boxPickRadius * 2.0);


                        fflush(eventFiles[8]);
                    }

                }

                //cout<<"wp r 1"<<endl;
            }

        }
        // put in the box
        if ((MatrlocalBoxCoord.norm() < boxPickRadius)&&(currentWaypoint == -1)){
            inBoxReach +=1;
            start_final = high_resolution_clock::now();
            box_trans->setScale(osg::Vec3d(0.8,0.8,0.8));
            if (inBoxReach==1){
                counter_start = high_resolution_clock::now();
                milliseconds enter_ms = duration_cast<milliseconds>(counter_start - trial_counter_start);
                cout<< "Start point is" <<enter_ms.count()<<endl;
                //sleep(10);
            }
            //subLevel = inBoxReach;

            // event: target engaged
            if (!targetEngaged)
            {
                auto tmpTargetTime = high_resolution_clock::now();
                milliseconds tmpMS = duration_cast<milliseconds>(tmpTargetTime - gameStartTime);
                float tmpTargetTimeFlt = float(tmpMS.count()/1000.0);
                timeTargetEngaged = tmpTargetTimeFlt;

                fwrite(&tmpTargetTimeFlt, sizeof(float), 1, eventFiles[2]);
                float tpx = box_trans->getPosition().x() / (boxPickRadius * 2.0);
                float tpy = box_trans->getPosition().y() / (boxPickRadius * 2.0);
                float tpz = box_trans->getPosition().z() / (boxPickRadius * 2.0);
                fwrite(&tpx, sizeof(float), 1, eventFiles[2]);
                fwrite(&tpy, sizeof(float), 1, eventFiles[2]);
                fwrite(&tpz, sizeof(float), 1, eventFiles[2]);
                fwrite(&trialNumber, sizeof(float), 1, eventFiles[2]);

                fflush(eventFiles[2]);
                targetEngaged = true;
            }

            bool openByEye = false;
           /* if (eyeMode) {
                cout<<" framesStareTarget: "<<framesStareTarget<<endl;
                if (eyeX != -1000) {

                    bioFeedBox->setElementAlpha(1.0);

                    float ex=0;
                    float ey=0;
                    ex = (0.5 - (eyeX / 1920.0)) * (320.0/10.0) + 2.0;
                    ey = (0.5 - (eyeY / 1080.0)) * (320.0/18.0) + 2.0;
                    /*cout<<"ex: "<<eyeX<<endl;
                    cout<<"targetPosIn2D.x(): "<<targetPosIn2D.x()<<endl;
                    cout<<"ey: "<<eyeY<<endl;
                    cout<<"ey: "<<1080.0-eyeY<<endl;
                    cout<<"targetPosIn2D.y(): "<<targetPosIn2D.y()<<endl;
                    cout<<"targetPosIn2D.z(): "<<targetPosIn2D.z()<<endl;*/
                    /*float eyeDisTarget = sqrt((eyeX - targetPosIn2D.x()) * (eyeX - targetPosIn2D.x()) +
                                              ((1080.0-eyeY) - targetPosIn2D.y()) * ((1080.0-eyeY) - targetPosIn2D.y()) );

                    cout<<"eyeDisTarget "<<eyeDisTarget<<endl;


                    if (eyeDisTarget < (90) ) {
                        framesStareTarget += 1;
                        framesNotStareTarget = 0;
                    } else {
                        framesNotStareTarget += 1;
                        if (framesNotStareTarget > 50) {
                            framesStareTarget = 0;
                        }
                    }

                    if (framesStareTarget > 140) {
                        openByEye = true;
                        framesStareTarget = 0;
                    }

                } else {
                    bioFeedBox->setElementAlpha(0.5);
                }
            }*/

            if ((true && openHand) || (!true && eegclick[0]) || (openByEye)) {
                //if (openByEye)
                //    openByEye = false;

                //If there was a click, send a zmq message with the time
                //get current time
                //When publishing to python everything is strings
                //if (demoMode && openHand) {
                if ( (inBoxReach > 0)&&(guiEventHandler->handState == true)) {
                    inBoxReach = 0;
                    auto release_time = high_resolution_clock::now();
                    milliseconds time_zmq_message = duration_cast<milliseconds>(release_time - counter_start);
                    message_arr[0] = time_zmq_message.count();
                    //stringstream message;
                    //message<<setprecision(3)<<fixed<<message_arr[0]/1000<<",";
                    //message<<endl;
                    //zmq::message_t histogram_message(message.str().length());
                    //memcpy((char *) histogram_message.data(), message.str().c_str(), message.str().length());
                    //publisher_histogram.send(histogram_message);
                    guiEventHandler->step = 0;
                    guiEventHandler->handState = false;
                    stickBallHand = false;
                    inReach = 0;
                    totalNumberOfDrops += 1.0;
                    // event: ball dropped
                    {
                        auto tmpBallTime = high_resolution_clock::now();
                        milliseconds tmpMS = duration_cast<milliseconds>(tmpBallTime - gameStartTime);
                        float tmpBallTimeFlt = float(tmpMS.count()/1000.0);

                        fwrite(&tmpBallTimeFlt, sizeof(float), 1, eventFiles[5]);

                        float bx = sph_trans->getPosition().x() / (boxPickRadius * 2.0);
                        float by = sph_trans->getPosition().y() / (boxPickRadius * 2.0);
                        float bz = sph_trans->getPosition().z() / (boxPickRadius * 2.0);
                        fwrite(&bx, sizeof(float), 1, eventFiles[5]);
                        fwrite(&by, sizeof(float), 1, eventFiles[5]);
                        fwrite(&bz, sizeof(float), 1, eventFiles[5]);

                        float distToTargetNorm = MatrTargetBoxCoord.norm() / (boxPickRadius * 2.0);
                        fwrite(&distToTargetNorm, sizeof(float), 1, eventFiles[5]);

                        float cor = 1.0;
                        fwrite(&cor, sizeof(float), 1, eventFiles[5]);

                        fflush(eventFiles[5]);
                    }

                    //thread tadath(tadaMusic);
                    //tadath.join();

                    if (firstTrial) {

                        firstTrial = false;
                    }

                    //tFile<<(double(end - absBegin) / CLOCKS_PER_SEC * 10.0)<<","<<elapsed_secs<<endl;
                    //tFile.flush();
                    //begin = clock();


                    sph_pos.x() = (double)rand()/(RAND_MAX)* 14.0 - 7.0;
                    sph_pos.y() = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
                    sph_pos.z() = (double)rand()/(RAND_MAX)* 10.0 - 5;
                    sph_trans->setPosition(sph_pos);

                    box_pos.x() = (double)rand()/(RAND_MAX)* 14.0 - 7.0;
                    box_pos.y() = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
                    box_pos.z() = (double)rand()/(RAND_MAX)* 10.0 - 5;

                    osg::Vec3f tmp = box_pos - sph_pos;
                    while (tmp.length() < 2.0) {
                        box_pos.x() = (double)rand()/(RAND_MAX)* 14.0 - 7.0;
                        box_pos.y() = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
                        box_pos.z() = (double)rand()/(RAND_MAX)* 10.0 - 5;

                        tmp = box_pos - sph_pos;
                    }

                    for (size_t i=0; i< numWaypoints; i++) {
                        double x = (double)rand()/(RAND_MAX)* 14.0 - 7.0;
                        double y = 0.0;//(double)rand()/(RAND_MAX)*(-2.0)-4.0;
                        double z = (double)rand()/(RAND_MAX)* 10.0 - 5;

                        wPoints[i]->setPosition(osg::Vec3d(x, y, z));
                    }

                    if (currentWaypoint == -1){
                        currentWaypoint = 0;
                        box_trans->setScale(osg::Vec3d(1.0,1.0,1.0));}


                    score += 10;
                    tmpTrial += 1;

                    {
                        auto tmpTargetTime = high_resolution_clock::now();
                        milliseconds tmpMS = duration_cast<milliseconds>(tmpTargetTime - gameStartTime);
                        float tmpTargetTimeFlt = float(tmpMS.count()/1000.0);

                        totalTimeTargetEngaged += (tmpTargetTimeFlt - timeTargetEngaged);
                    }

                    {
                        auto tmpTime = high_resolution_clock::now();
                        milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - withBallTDTimer);
                        float tmpTimeFlt = float(tmpMS.count()/1000.0);

                        // was holding the ball add time to total
                        totalTimeWithBall += tmpTimeFlt;
                    }

                    totalTimeWithBall;
                    // event: current trial is over
                    {
                        auto tmpTrialTime = high_resolution_clock::now();
                        milliseconds tmpMS = duration_cast<milliseconds>(tmpTrialTime - gameStartTime);
                        float tmpTrialTimeFlt = float(tmpMS.count()/1000.0);

                        // don't count the last drop
                        totalNumberOfDrops -= 1;

                        float withBallNotIntarget = totalTimeWithBall - totalTimeTargetEngaged;

                        fwrite(&tmpTrialTimeFlt, sizeof(float), 1, eventFiles[1]);
                        fwrite(&totalTimeWithBall, sizeof(float), 1, eventFiles[1]);
                        fwrite(&totalTimeWithoutBall, sizeof(float), 1, eventFiles[1]);
                        fwrite(&totalTimeTargetEngaged, sizeof(float), 1, eventFiles[1]);
                        fwrite(&withBallNotIntarget, sizeof(float), 1, eventFiles[1]);
                        fwrite(&totalNumberOfDrops, sizeof(float), 1, eventFiles[1]);

                        float openByEyeFlt = float(openByEye);
                        fwrite(&openByEyeFlt, sizeof(float), 1, eventFiles[1]);

                        fflush(eventFiles[1]);
                    }
                    totalTimeWithoutBall = 0.0;
                    totalTimeWithBall = 0.0;
                    totalTimeTargetEngaged = 0.0;
                    startTargetEngagedTimer = false;
                    totalNumberOfDrops = 0.0;
                    targetEngaged = false;
                    withBallTDTimer = high_resolution_clock::now();
                    withoutBallTDTimer = high_resolution_clock::now();

                    // event: a new trial started
                    trialNumber += 1.0;
                    auto tmpTrialTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpTrialTime - gameStartTime);
                    float tmpTrialTimeFlt = float(tmpMS.count()/1000.0);
                    stickBallHand = true;
                    guiEventHandler->step = 0;
                    guiEventHandler->handState = true;

                    fwrite(&tmpTrialTimeFlt, sizeof(float), 1, eventFiles[0]);
                    fwrite(&trialNumber, sizeof(float), 1, eventFiles[0]);
                    fflush(eventFiles[0]);


                    trial_counter_start = high_resolution_clock::now();
                    if (tmpTrial > trialPerSublevel) {
                        tmpTrial = 1;
                        subLevel += 1;
                        start = high_resolution_clock::now();
                        new_duration = base_duration;


                    }
                    if (subLevel > sublevelPerLevel) {
                        level += 1;
                        subLevel = 1;
                        subLevelFlag = true;

                        //new_duration = base_duration*level;
                    }

                    box_trans->setPosition(box_pos);

                }
            }
        }

        else if(currentWaypoint == -1) {
            //if(eyeMode) {
            //    framesStareTarget = 0;
            //}
            if (inBoxReach > 0) {
                // event: target disengaged
                targetEngaged = false;
                {
                    auto tmpTargetTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpTargetTime - gameStartTime);
                    float tmpTargetTimeFlt = float(tmpMS.count()/1000.0);

                    totalTimeTargetEngaged += (tmpTargetTimeFlt - timeTargetEngaged);

                    fwrite(&tmpTargetTimeFlt, sizeof(float), 1, eventFiles[3]);
                    fwrite(&trialNumber, sizeof(float), 1, eventFiles[3]);
                    fflush(eventFiles[3]);
                }

            }
            inBoxReach = 0;
            box_trans->setScale(osg::Vec3d(1.3,1.3,1.3));
            // open hand release the ball if is asked to
            if ((openHand && stickBallHand)||( eegclick[0] && stickBallHand)) {
                droput_clock = high_resolution_clock::now();
                stickBallHand = false;
                inReach = 0;
                totalNumberOfDrops += 1.0;
                // event: ball dropped
                {
                    auto tmpBallTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpBallTime - gameStartTime);
                    float tmpBallTimeFlt = float(tmpMS.count()/1000.0);

                    fwrite(&tmpBallTimeFlt, sizeof(float), 1, eventFiles[5]);

                    float bx = sph_trans->getPosition().x() / (boxPickRadius * 2.0);
                    float by = sph_trans->getPosition().y() / (boxPickRadius * 2.0);
                    float bz = sph_trans->getPosition().z() / (boxPickRadius * 2.0);
                    fwrite(&bx, sizeof(float), 1, eventFiles[5]);
                    fwrite(&by, sizeof(float), 1, eventFiles[5]);
                    fwrite(&bz, sizeof(float), 1, eventFiles[5]);

                    float distToTargetNorm = MatrTargetBoxCoord.norm() / (boxPickRadius * 2.0);
                    fwrite(&distToTargetNorm, sizeof(float), 1, eventFiles[5]);

                    float cor = 0.0;
                    fwrite(&cor, sizeof(float), 1, eventFiles[5]);

                    fflush(eventFiles[5]);
                }

                guiEventHandler->handState = false;
                guiEventHandler->step = 0;
                inReach = 0;
            }
        } else {
            //check if there is a drop midway
            //reset the drop clock

            // open hand release the ball if is asked to
            if ((openHand || eegclick[0]) && stickBallHand) {
                droput_clock = high_resolution_clock::now();
                stickBallHand = false;
                inReach = 0;

                totalNumberOfDrops += 1.0;
                // event: ball dropped
                {
                    auto tmpBallTime = high_resolution_clock::now();
                    milliseconds tmpMS = duration_cast<milliseconds>(tmpBallTime - gameStartTime);
                    float tmpBallTimeFlt = float(tmpMS.count()/1000.0);

                    fwrite(&tmpBallTimeFlt, sizeof(float), 1, eventFiles[5]);

                    float bx = sph_trans->getPosition().x()  / (boxPickRadius * 2.0);
                    float by = sph_trans->getPosition().y()  / (boxPickRadius * 2.0);
                    float bz = sph_trans->getPosition().z()  / (boxPickRadius * 2.0);
                    fwrite(&bx, sizeof(float), 1, eventFiles[5]);
                    fwrite(&by, sizeof(float), 1, eventFiles[5]);
                    fwrite(&bz, sizeof(float), 1, eventFiles[5]);

                    float distToTargetNorm = MatrTargetBoxCoord.norm() / (boxPickRadius * 2.0);
                    fwrite(&distToTargetNorm, sizeof(float), 1, eventFiles[5]);

                    float cor = 0.0;
                    fwrite(&cor, sizeof(float), 1, eventFiles[5]);

                    fflush(eventFiles[5]);
                }

                guiEventHandler->handState = false;
                guiEventHandler->step = 0;
            }
        }

        if (openHand)
            openHand = false;

        if (targetEngaged) {
            // event: compute total enaged time
            if (!startTargetEngagedTimer) {
                startTargetEngagedTimer = true;
                targetEngagedTimer = high_resolution_clock::now();
            }
        } else if (startTargetEngagedTimer) {
            startTargetEngagedTimer = false;
            // target is not enaged anymore, compute interval and add to total time
            auto tmpTime = high_resolution_clock::now();
            milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - targetEngagedTimer);
            float tmpTimeFlt = float(tmpMS.count()/1000.0);
            //totalTimeTargetEngaged += tmpTimeFlt;
        }

        /*if (targetEngaged) {
            {
              auto tmpTime = high_resolution_clock::now();
              milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - withoutBallTDTimer);
              float tmpTimeFlt = float(tmpMS.count()/1000.0);
              totalTimeWithoutBall += tmpTimeFlt;
            }
            {
              auto tmpTime = high_resolution_clock::now();
              milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - withBallTDTimer);
              float tmpTimeFlt = float(tmpMS.count()/1000.0);
              totalTimeWithBall += tmpTimeFlt;

            }

            withBallTDTimer = high_resolution_clock::now();
            withoutBallTDTimer = high_resolution_clock::now();
        }*/

        if (stickBallHand) {
            osg::Vec3 centB = osg::Vec3(0.053, 0.0, 0.0) * m.front();
            sph_pos = centB;
            sph_trans->setPosition(sph_pos);

            // event: compute total time holding and not holding the ball
            if (!wasHoldingBall) {
                wasHoldingBall = true;

                withBallTDTimer = high_resolution_clock::now();

                auto tmpTime = high_resolution_clock::now();
                milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - withoutBallTDTimer);
                float tmpTimeFlt = float(tmpMS.count()/1000.0);

                // wasn't holding the ball add time to total
                totalTimeWithoutBall += tmpTimeFlt;
            }

        } else if (wasHoldingBall) {
            wasHoldingBall = false;

            withoutBallTDTimer = high_resolution_clock::now();

            auto tmpTime = high_resolution_clock::now();
            milliseconds tmpMS = duration_cast<milliseconds>(tmpTime - withBallTDTimer);
            float tmpTimeFlt = float(tmpMS.count()/1000.0);

            // was holding the ball add time to total
            totalTimeWithBall += tmpTimeFlt;
        }

        //cout<<"totalTimeWithBall: "<<totalTimeWithBall<<endl;
        //cout<<"totalTimeWithoutBall: "<<totalTimeWithoutBall<<endl;


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

