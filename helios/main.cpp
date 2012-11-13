#include <iostream>

#include <osg/Texture2D>
#include <osg/ImageStream>
#include <osg/TextureRectangle>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>

#include "nextTimeStep.h"
#include "parameters.h"
#include <time.h>
#include <map>
#include <armadillo>

#include "pelops.h"
#include "tweakbargui.h"
//#include <opencv2/opencv.hpp>

using namespace osg;
using namespace std;
using namespace arma;

osg::ShapeDrawable* unitSphereDrawable_mob;

osgViewer::Viewer viewer;
osg::Vec3 pointer_pose(0,0,0);
osg::PositionAttitudeTransform* sphereXForm_mob = new osg::PositionAttitudeTransform();

ExportState export_state;

//cv::VideoCapture cap1(0);
//cv::VideoCapture cap2(1);

ofstream eegfile;

// FILE* gfeed = popen("./feedgnuplot --lines --stream -xlen 2000", "w");

timespec diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

mat baselineSamples = zeros<mat>(N_BASELINE_SAMPLES, N_CHANNELS);
int baselineCounter = 0;
int trialCounter = 0;
vec averages = zeros<vec>(N_CHANNELS);
map<string, cube> graphList, trainGraphList, testGraphList;

size_t sample_counter = 0;

void test_func(vec samples) {

//    eegfile<<samples.t()<<endl;
//    cv::Mat frame1,frame2;
//    cap1 >> frame1;
//    stringstream tss;
//    tss<<sample_counter;
//    cv::imwrite(("cam1_"+tss.str()+".png").c_str(),frame1);
//    cap2 >> frame2;
//    cv::imwrite(("cam2_"+tss.str()+".png").c_str(),frame2);
//    sample_counter += 1;

    timespec startTimer, endTimer;
    clock_gettime(CLOCK_REALTIME, &startTimer);
    // cout<<samples.t()<<endl;
    //vec tmp_samples(64);
    // fprintf(gfeed, "%f \n", samples[1]);
    // fprintf(gfeed, "replot\n");
    // fflush(gfeed);
    if (baselineCounter < N_BASELINE_SAMPLES) {
        baselineSamples.row(baselineCounter) = \
                trans(samples.subvec(0, N_CHANNELS - 1));
        baselineCounter++;

        if(baselineCounter == N_BASELINE_SAMPLES)
        {
            averages = vec(trans(mean(baselineSamples, 0)));
            CHANNEL_VARIANCES = trans(var(baselineSamples, 0, 0));
            cout<<"averages "<<averages<<endl;
            cout<<"variances "<<CHANNEL_VARIANCES<<endl;
        }

        viewer.frame();
    }

    else
    {
        vec pos = zeros<vec>(3);

        int instruction = 0;
        nextTimeStep(pos, instruction, graphList, \
                     samples.subvec(0, N_CHANNELS - 1) - averages);

        cout<<"pos: "<<pos<<endl;

        if (instruction == -1)
        {
            cout<<"Terminated Successfully"<<endl;
            graphList["channelParametersCube"].save("channelParametersCube.txt", raw_ascii);
            graphList["meanInnovationCube"].save("meanInnovationCube.txt", raw_ascii);

            ofstream channelParamsFile("channelParametersCube2.txt");
            channelParamsFile<<graphList["channelParametersCube"];

            ofstream meanInnovationCubeFile("meanInnovationCube2.txt");
            meanInnovationCubeFile<<graphList["meanInnovationCube"];

            cout<<"meanInnovationCube: "<<graphList["meanInnovationCube"]<<endl;

            channelParamsFile.flush();
            meanInnovationCubeFile.flush();

            exit(0);
        }
        else if (instruction == 0)
        {
            cout<<"continue"<<endl;
            clock_gettime(CLOCK_REALTIME, &endTimer);
            timespec diffTime = diff(startTimer, endTimer);
            while(diffTime.tv_sec * 1e9 + diffTime.tv_nsec < TIME_BIN * 1e9)
            {
                clock_gettime(CLOCK_REALTIME, &endTimer);
                diffTime = diff(startTimer, endTimer);
            }

            double realTimeBinSize = \
                    (diffTime.tv_sec * 1.0e9 +diffTime.tv_nsec) / 1.0e9;

            pointer_pose = osg::Vec3(pos(0)*10.0,pos(1)*10.0,pos(2)*10.0);
            sphereXForm_mob->setPosition(pointer_pose);
            viewer.frame();
        }
        else
        {
            if(TEST_TRIALS[trialCounter])
            {
                stringstream ss;
                ss<<trialCounter;
                export_state.status = string("Test ") + ss.str();
                graphList = testGraphList;

            }
            else
            {
                stringstream ss;
                ss<<trialCounter;
                export_state.status = string("Train ") + ss.str();
                graphList = trainGraphList;
            }
            trialCounter++;

            cout<<"wait: "<<instruction<<endl;
            unitSphereDrawable_mob->setColor( osg::Vec4( 1,0,0,1 ));
            pointer_pose = osg::Vec3(pos(0)*10.0,pos(1)*10.0,pos(2)*10.0);
            sphereXForm_mob->setPosition(pointer_pose);
            viewer.frame();
            sleep(instruction);
            unitSphereDrawable_mob->setColor( osg::Vec4( 1,1,1,1 ));
            viewer.frame();
        }

    }
}

int main(int argc, char **argv) {

    eegfile.open("eeg.txt");
    //viewer.setUpViewInWindow(0,0,800, 600);
    osg::DisplaySettings::instance()->setNumMultiSamples( 4 );

    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::ThreadingHandler);
    //viewer.getCamera()->setClearMask(GL_DEPTH_BUFFER_BIT);

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);


    osg::Group* root = new Group();
    Geode* pyramidGeode = new Geode();
    Geometry* pyramidGeometry = new Geometry();
    
    pyramidGeode->addDrawable(pyramidGeometry);
    //root->addChild(pyramidGeode);
    
    osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
    pyramidVertices->push_back( osg::Vec3( 0, 0, 0) ); // front left
    pyramidVertices->push_back( osg::Vec3(10, 0, 0) ); // front right
    pyramidVertices->push_back( osg::Vec3(10,10, 0) ); // back right
    pyramidVertices->push_back( osg::Vec3( 0,10, 0) ); // back left
    pyramidVertices->push_back( osg::Vec3( 5, 5,10) ); // peak


    pyramidGeometry->setVertexArray( pyramidVertices );


    osg::DrawElementsUInt* pyramidBase =
        new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    pyramidBase->push_back(3);
    pyramidBase->push_back(2);
    pyramidBase->push_back(1);
    pyramidBase->push_back(0);
    pyramidGeometry->addPrimitiveSet(pyramidBase);

    //Repeat the same for each of the four sides. Again, vertices are
    //specified in counter-clockwise order.

    osg::DrawElementsUInt* pyramidFaceOne =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    pyramidFaceOne->push_back(0);
    pyramidFaceOne->push_back(1);
    pyramidFaceOne->push_back(4);
    pyramidGeometry->addPrimitiveSet(pyramidFaceOne);

    osg::DrawElementsUInt* pyramidFaceTwo =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    pyramidFaceTwo->push_back(1);
    pyramidFaceTwo->push_back(2);
    pyramidFaceTwo->push_back(4);
    pyramidGeometry->addPrimitiveSet(pyramidFaceTwo);

    osg::DrawElementsUInt* pyramidFaceThree =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    pyramidFaceThree->push_back(2);
    pyramidFaceThree->push_back(3);
    pyramidFaceThree->push_back(4);
    pyramidGeometry->addPrimitiveSet(pyramidFaceThree);

    osg::DrawElementsUInt* pyramidFaceFour =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    pyramidFaceFour->push_back(3);
    pyramidFaceFour->push_back(0);
    pyramidFaceFour->push_back(4);
    pyramidGeometry->addPrimitiveSet(pyramidFaceFour);

    //Declare and load an array of Vec4 elements to store colors.

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
    colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) ); //index 1 green
    colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f) ); //index 2 blue
    colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); //index 3 white
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 4 red

    //The next step is to associate the array of colors with the geometry,
    //assign the color indices created above to the geometry and set the
    //binding mode to _PER_VERTEX.

    pyramidGeometry->setColorArray(colors);
    pyramidGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    //Now that we have created a geometry node and added it to the scene
    //we can reuse this geometry. For example, if we wanted to put a
    //second pyramid 15 units to the right of the first one, we could add
    //this geode as the child of a transform node in our scene graph.

    // Declare and initialize a transform node.
    osg::PositionAttitudeTransform* pyramidTwoXForm =
        new osg::PositionAttitudeTransform();

    // Use the 'addChild' method of the osg::Group class to
    // add the transform as a child of the root node and the
    // pyramid node as a child of the transform.

    //root->addChild(pyramidTwoXForm);
    pyramidTwoXForm->addChild(pyramidGeode);
    
    osg::Sphere* unitSphere = new osg::Sphere( osg::Vec3(-5.5,0,0), 0.3);
    osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(unitSphere);
    osg::PositionAttitudeTransform* sphereXForm = new osg::PositionAttitudeTransform();
    sphereXForm->setPosition(osg::Vec3(5.5,0,0));
    osg::Geode* unitSphereGeode = new osg::Geode();
    root->addChild(sphereXForm);
    sphereXForm->addChild(unitSphereGeode);
    unitSphereGeode->addDrawable(unitSphereDrawable);

    osg::Sphere* unitSphere_mob = new osg::Sphere( osg::Vec3(0,0,0), 0.15);
    unitSphereDrawable_mob = new osg::ShapeDrawable(unitSphere_mob);

    sphereXForm_mob->setPosition(osg::Vec3(5.5,0,0));
    osg::Geode* unitSphereGeode_mob = new osg::Geode();
    root->addChild(sphereXForm_mob);
    sphereXForm_mob->addChild(unitSphereGeode_mob);
    unitSphereGeode_mob->addDrawable(unitSphereDrawable_mob);

    //osg::Box* box = new osg::Box(Vec3(0,0,0),10, 10, 10);
    //osg::
    float scale=0.2;
    float scale2=0.1;
    osg::ShapeDrawable* pBox = new osg::ShapeDrawable( new osg::Box(osg::Vec3(15.0f*scale,15.0f*scale,15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox2 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(-15.0f*scale,-15.0f*scale,-15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox3 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(15.0f*scale,15.0f*scale,-15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox4 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(15.0f*scale,-15.0f*scale,-15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );

    osg::ShapeDrawable* pBox5 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(-15.0f*scale,15.0f*scale,-15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox6 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(15.0f*scale,-15.0f*scale,15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox7 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(-15.0f*scale,15.0f*scale,15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );
    osg::ShapeDrawable* pBox8 = new osg::ShapeDrawable( new osg::Box(osg::Vec3(-15.0f*scale,-15.0f*scale,15.0f*scale),1.0f*scale2, 2.0f, 1.0f*scale2) );

    osg::Geode* box_geode = new osg::Geode();
    osg::Geode* box_geode2 = new osg::Geode();
    osg::Geode* box_geode3 = new osg::Geode();
    osg::Geode* box_geode4 = new osg::Geode();
    osg::Geode* box_geode5 = new osg::Geode();
    osg::Geode* box_geode6 = new osg::Geode();
    osg::Geode* box_geode7 = new osg::Geode();
    osg::Geode* box_geode8 = new osg::Geode();
    root->addChild(box_geode);
    root->addChild(box_geode2);
    root->addChild(box_geode3);
    root->addChild(box_geode4);
    root->addChild(box_geode5);
    root->addChild(box_geode6);
    root->addChild(box_geode7);
    root->addChild(box_geode8);
    box_geode->addDrawable(pBox);
    box_geode2->addDrawable(pBox2);
    box_geode3->addDrawable(pBox3);
    box_geode4->addDrawable(pBox4);
    box_geode5->addDrawable(pBox5);
    box_geode6->addDrawable(pBox6);
    box_geode7->addDrawable(pBox7);
    box_geode8->addDrawable(pBox8);

    unitSphereDrawable->setColor( osg::Vec4( 0,1,0,0.5 ));

    osg::StateSet *state = unitSphereDrawable->getOrCreateStateSet();
    osg::PolygonMode *polyModeObj;
    polyModeObj = dynamic_cast< osg::PolygonMode* > ( state->getAttribute( osg::StateAttribute::POLYGONMODE ));
    if ( !polyModeObj ) {
        polyModeObj = new osg::PolygonMode;
        state->setAttribute( polyModeObj );
       }

     polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

     osg::StateSet *state2 = unitSphereDrawable_mob->getOrCreateStateSet();
     polyModeObj = dynamic_cast< osg::PolygonMode* > ( state2->getAttribute( osg::StateAttribute::POLYGONMODE ));
     if ( !polyModeObj ) {
         polyModeObj = new osg::PolygonMode;
         state2->setAttribute( polyModeObj );
        }

      polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

//      state2 = pBox->getOrCreateStateSet();
//      polyModeObj = dynamic_cast< osg::PolygonMode* > ( state2->getAttribute( osg::StateAttribute::POLYGONMODE ));
//      if ( !polyModeObj ) {
//          polyModeObj = new osg::PolygonMode;
//          state2->setAttribute( polyModeObj );
//         }

//       polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

       // Declare and initialize a Vec3 instance to change the
    // position of the model in the scene

    osg::Vec3 pyramidTwoPosition(15,0,0);
    pyramidTwoXForm->setPosition( pyramidTwoPosition );

    // switch off lighting as we haven't assigned any normals.
    root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    // Tweak Bar GUI
    osg::ref_ptr<osg::Geode> tb_geode = new osg::Geode;
    osg::ref_ptr<TweakBarDrawable> cd = new TweakBarDrawable();
    tb_geode->addDrawable(cd.get());
    osg::StateSet* ss = tb_geode->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Camera* hudCam = cd->createHUD(800, 600);
    hudCam->addChild(tb_geode);

    root->addChild(hudCam);

    //The final step is to set up and enter a simulation loop.

    viewer.setSceneData( root );
    //viewer.run();

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.realize();
    viewer.getCamera()->getGraphicsContext()->makeCurrent();

    TweakBarEventCallback* e_handler;
    e_handler = new TweakBarEventCallback(&export_state);
    viewer.addEventHandler(e_handler);

    // Kevin's code
    InitFilter();

    // EEG
    LinAmp lamp(test_func);
    if (lamp.Init() == false) {
        cout<<"Failed to initialize the device"<<endl;
        return 1;
    }

    lamp.StartSampling();
    vec samples = zeros<vec>(64);

    while( !viewer.done() )
    {
        sphereXForm_mob->setPosition(pointer_pose);
        viewer.frame();
        test_func(samples);
    }

    lamp.StopSampling();

    return 0;
}
