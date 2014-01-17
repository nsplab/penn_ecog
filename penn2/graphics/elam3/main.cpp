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

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <osg/Switch>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/NodeVisitor>
#include <osg/ShapeDrawable>
#include <osgFX/Effect>

#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osg/PointSprite>
#include <osg/Point>

//tmp
#include <osg/ShapeDrawable>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

#include <zmq.hpp>

#include "utils.h"

using namespace std;
using namespace zmq;

extern bool pauseGame;

osgParticle::ParticleSystem* createFireParticles(
        osg::Group* parent ) {
    osg::ref_ptr<osgParticle::ParticleSystem> ps =
    new osgParticle::ParticleSystem;
    ps->getDefaultParticleTemplate().setLifeTime( 1.5f );
    ps->getDefaultParticleTemplate().setShape(
    osgParticle::Particle::QUAD );
    ps->getDefaultParticleTemplate().setSizeRange(
    osgParticle::rangef(1.0f, 0.1f) );
    ps->getDefaultParticleTemplate().setAlphaRange(
    osgParticle::rangef(1.0f, 0.5f) );
    ps->getDefaultParticleTemplate().setColorRange(
    osgParticle::rangev4(osg::Vec4(0.1f,0.3f,0.4f,1.0f),
    osg::Vec4(0.0f,0.4f,0.4f,1.0f)) );
    ps->setDefaultAttributes("smoke.rgb", true, false );

    osg::ref_ptr<osgParticle::RandomRateCounter> rrc =
    new osgParticle::RandomRateCounter;
    rrc->setRateRange( 30, 50 );
    osg::ref_ptr<osgParticle::RadialShooter> shooter =
    new osgParticle::RadialShooter;
    shooter->setThetaRange( -osg::PI_4/2.0, osg::PI_4/2.0 );
    shooter->setPhiRange( -osg::PI_4/2.0, osg::PI_4/2.0 );
    shooter->setInitialSpeedRange( 5.0f, 7.5f );

    osg::ref_ptr<osgParticle::ModularEmitter> emitter =
    new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );
    emitter->setCounter( rrc.get() );
    emitter->setShooter( shooter.get() );
    parent->addChild( emitter.get() );
    return ps.get();
}

osg::Geode* createAxis(float scale = 10.0)
{
    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec4Array* colors   = new osg::Vec4Array();

    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(scale, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, scale, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.0f, 0.0f, scale));

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

osg::Geode* createMeshCube(float scale = 5.0f, float depth = 5.0f)
{
    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec4Array* colors   = new osg::Vec4Array();
    float r = 0.1;
    float g = 0.1;
    float b = 0.1;

    unsigned density = 5;
    unsigned depthDensity = density;

    // west
    /*for (unsigned i=0; i<=density; i++) {
        vertices->push_back(osg::Vec3(0.0f, scale*float(i)/density, depth/2.0));
        vertices->push_back(osg::Vec3(0.0f, scale*float(i)/density, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    for (unsigned i=0; i<=depthDensity; i++) {
        vertices->push_back(osg::Vec3(0.0f, 0.0f, -depth*float(i)/depthDensity+depth/2.0));
        vertices->push_back(osg::Vec3(0.0f, scale, -depth*float(i)/depthDensity+depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }*/

    // east
    for (unsigned i=0; i<=density; i++) {
        vertices->push_back(osg::Vec3(scale, scale*float(i)/density, depth/2.0));
        vertices->push_back(osg::Vec3(scale, scale*float(i)/density, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    for (unsigned i=0; i<=depthDensity; i++) {
        vertices->push_back(osg::Vec3(scale, 0.0f, -depth*float(i)/depthDensity+depth/2.0));
        vertices->push_back(osg::Vec3(scale, scale, -depth*float(i)/depthDensity+depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    // south
    for (unsigned i=0; i<=density; i++) {
        vertices->push_back(osg::Vec3(scale*float(i)/density, 0.0f, depth/2.0));
        vertices->push_back(osg::Vec3(scale*float(i)/density, 0.0f, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    for (unsigned i=0; i<=depthDensity; i++) {
        vertices->push_back(osg::Vec3(0.0f, 0.0f, -depth*float(i)/depthDensity+depth/2.0));
        vertices->push_back(osg::Vec3(scale, 0.0f, -depth*float(i)/depthDensity+depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }


    // north
    for (unsigned i=0; i<=density; i++) {
        vertices->push_back(osg::Vec3(scale*float(i)/density, scale, depth/2.0));
        vertices->push_back(osg::Vec3(scale*float(i)/density, scale, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    for (unsigned i=0; i<=depthDensity; i++) {
        vertices->push_back(osg::Vec3(0.0f, scale, -depth*float(i)/depthDensity+depth/2.0));
        vertices->push_back(osg::Vec3(scale, scale, -depth*float(i)/depthDensity+depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    // bottom
    for (unsigned i=0; i<=density; i++) {
        // vertical
        vertices->push_back(osg::Vec3(scale*float(i)/density, 0.0f, -depth/2.0));
        vertices->push_back(osg::Vec3(scale*float(i)/density, scale, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));

        // horizontal
        vertices->push_back(osg::Vec3(0.0f, scale*float(i)/density, -depth/2.0));
        vertices->push_back(osg::Vec3(scale, scale*float(i)/density, -depth/2.0));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
        colors->push_back(osg::Vec4(r, g, b, 1.0f));
    }

    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertices->size()-1));
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, false);

    geode->addDrawable(geometry);

    return geode;
}

class TransparencyTechnique : public osgFX::Technique
{
public:
    TransparencyTechnique() : osgFX::Technique() {}
    virtual bool validate(osg::State& ss) const {
        return true;
    }
protected:
    virtual void define_passes() {
        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
        ss->setAttributeAndModes( new osg::ColorMask(
        false, false, false, false) );
        ss->setAttributeAndModes( new osg::Depth(osg::Depth::LESS) );
        addPass( ss.get() );

        ss = new osg::StateSet;
        ss->setAttributeAndModes( new osg::ColorMask(
        true, true, true, true) );
        ss->setAttributeAndModes( new osg::Depth(osg::Depth::EQUAL) );
        addPass( ss.get() );
    }
};

class TransparencyNode : public osgFX::Effect
{
public:
    TransparencyNode() : osgFX::Effect() {}
    TransparencyNode( const TransparencyNode& copy,
                      const osg::CopyOp op=osg::CopyOp::SHALLOW_COPY )
        : osgFX::Effect(copy, op) {}
    META_Effect( osgFX, TransparencyNode, "TransparencyNode", "", "");
protected:
    virtual bool define_techniques() {
        addTechnique(new TransparencyTechnique);
        return true;
    }
};

class MakeTransparent:public osg::NodeVisitor
{
public:
    MakeTransparent(float alpha):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), alpha_(alpha) {
        setNodeMaskOverride(0xffffffff);
        setTraversalMask(0xffffffff);
    }
    using osg::NodeVisitor::apply;
    void apply(osg::Geode& geode) {
        for (int i = 0; i< geode.getNumDrawables(); i++) {
            osg::Drawable* dr = geode.getDrawable(i);
            osg::StateSet* state = dr->getOrCreateStateSet();
            osg::ref_ptr<osg::Material> mat = dynamic_cast<osg::Material*>(state->getAttribute(osg::StateAttribute::MATERIAL));
            mat->setAlpha(osg::Material::FRONT_AND_BACK, alpha_);
            state->setAttributeAndModes(mat.get(),
                                        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
                                                    osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
            state->setAttributeAndModes(bf);
            dr->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
    }
private:
    float alpha_;
};

float SolveArmInvKinematics(Eigen::Vector3f targetPos, Eigen::Vector3f& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat) {
    float error = 1.0;

    Eigen::Vector3f elbowAxisZ(0.0, 0.0, 1.0);
    Eigen::Vector3f shoulderAxisX(1.0, 0.0, 0.0);
    Eigen::Vector3f shoulderAxisY(0.0, 1.0, 0.0);
    Eigen::Vector3f shoulderAxisZ(0.0, 0.0, 1.0);

    Eigen::Vector3f handPos(1.50601, -0.23667, -0.30729);
    Eigen::Vector3f shoulderPos(0.0, 0.0, -2.0);
    Eigen::Vector3f elbowPos(-4.92282, -0.1, -2.0);
    Eigen::Vector3f wristPos(-8.78673, -0.09678, 0.04826 - 2.0);

    Eigen::Vector3f currentShoulderAxisX = shoulderQuat._transformVector(shoulderAxisX);
    Eigen::Vector3f currentShoulderAxisY = shoulderQuat._transformVector(shoulderAxisY);
    Eigen::Vector3f currentShoulderAxisZ = shoulderQuat._transformVector(shoulderAxisZ);
    Eigen::Vector3f currentElbowAxisZ = shoulderQuat._transformVector(elbowAxisZ);

    Eigen::Vector3f currentElbowPos = shoulderQuat._transformVector(elbowPos);
    Eigen::Vector3f currentWristPos = shoulderQuat._transformVector(wristPos);
    currentWristPos -= currentElbowPos;
    currentWristPos = elbowQuat._transformVector(currentWristPos);
    currentWristPos += currentElbowPos;


    auto start = chrono::high_resolution_clock::now();
    // ik 2 iterate until error is small
    while (error > 0.01) {

        // ik 3 get current orientations and positions

        // ik 4 compute jacobian
        Eigen::Vector3f jacobCol1 = currentShoulderAxisX.cross(currentWristPos - shoulderPos);
        Eigen::Vector3f jacobCol2 = currentShoulderAxisY.cross(currentWristPos - shoulderPos);
        Eigen::Vector3f jacobCol3 = currentShoulderAxisZ.cross(currentWristPos - shoulderPos);
        Eigen::Vector3f jacobCol4 = currentElbowAxisZ.cross(currentWristPos - currentElbowPos);

        Eigen::Matrix<float, 3, 4> jacobian;
        jacobian.col(0) = jacobCol1;
        jacobian.col(1) = jacobCol2;
        jacobian.col(2) = jacobCol3;
        jacobian.col(3) = jacobCol4;

        // target - currect position
        Eigen::Vector3f displacement = targetPos - currentWristPos;

        Eigen::Vector3f jjtd = jacobian * jacobian.transpose() * displacement;

        float alpha = displacement.dot(jjtd) / jjtd.dot(jjtd) * 0.01;
        //cout<<"alpha "<<alpha<<endl;

        // ik 5 compute rotation updates
        Eigen::Vector4f deltaTheta = alpha * jacobian.transpose() * displacement;
        //cout<<"deltaTheta "<<deltaTheta<<endl;

        // ik 5 update orientations
        Eigen::Quaternion<float> deltaShoulderQuat =
                //Eigen::AngleAxis<float>(deltaTheta(0), currentShoulderAxisX) *
                Eigen::AngleAxis<float>(deltaTheta(1), currentShoulderAxisY) *
                //Eigen::AngleAxis<float>(0.0, currentShoulderAxisZ);
                Eigen::AngleAxis<float>(deltaTheta(2), currentShoulderAxisZ);
        shoulderQuat = deltaShoulderQuat * shoulderQuat;

        Eigen::Quaternion<float> deltaElbowQuat (Eigen::AngleAxis<float>(deltaTheta(3), currentElbowAxisZ));
        elbowQuat = deltaElbowQuat * elbowQuat;

        currentShoulderAxisX = shoulderQuat._transformVector(shoulderAxisX);
        currentShoulderAxisY = shoulderQuat._transformVector(shoulderAxisY);
        currentShoulderAxisZ = shoulderQuat._transformVector(shoulderAxisZ);
        currentElbowAxisZ = shoulderQuat._transformVector(elbowAxisZ);
        //cout<<"currentElbowAxisZ "<<currentElbowAxisZ<<endl;
        //cout<<"elbowQuat "<<elbowQuat.w()<<" "<<elbowQuat.x()<<" "<<elbowQuat.y()<<" "<<elbowQuat.z()<<endl;


        currentElbowPos = shoulderQuat._transformVector(elbowPos);

        currentWristPos = shoulderQuat._transformVector(wristPos);
        currentWristPos -= currentElbowPos;
        currentWristPos = elbowQuat._transformVector(currentWristPos);
        currentWristPos += currentElbowPos;

        displacement = targetPos - currentWristPos;
        error = displacement.norm();
        //cout<<"error "<<error<<endl;
        //this_thread::sleep_for(chrono::milliseconds(500));

    }
    auto end = chrono::high_resolution_clock::now();
    cout<<chrono::duration_cast<chrono::nanoseconds>(end-start).count()<<"ns\n";

    currentPos = currentWristPos;

    return error;
}


int main()
{
    // root of scene graph
    osg::ref_ptr<osg::Group> root = new osg::Group();

    // load upper arm model
    osg::ref_ptr<osg::Node> upperArm = osgDB::readNodeFile("./upper_arm.3ds");

    // load upper arm model
    osg::ref_ptr<osg::Node> foreArm = osgDB::readNodeFile("./fore_arm.3ds");

    MakeTransparent mkTransparent(0.6);
    upperArm->accept(mkTransparent);
    foreArm->accept(mkTransparent);

    osg::ref_ptr<osg::PositionAttitudeTransform> patUpperArm = new osg::PositionAttitudeTransform();
    patUpperArm->setPosition(osg::Vec3(0,0,-2.0));
    patUpperArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(3.141592, osg::Vec3d(0,0,1)));

    osg::ref_ptr<osg::PositionAttitudeTransform> patForeArm = new osg::PositionAttitudeTransform();
    patForeArm->setPosition(osg::Vec3(0,0,-2.0));
    patForeArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(0, osg::Vec3d(0,0,1)));

    patUpperArm->addChild(upperArm);

    osg::ref_ptr<osg::ShapeDrawable> sphereHand = new osg::ShapeDrawable;
    sphereHand->setShape( new osg::Sphere(osg::Vec3(-3.8, -0.107, 0.03077), 0.4f) );
    osg::ref_ptr<osg::Geode> sphere = new osg::Geode;
    sphere->addDrawable(sphereHand.get());
    patForeArm->addChild(sphere);

    patForeArm->addChild(foreArm);
    patUpperArm->addChild(patForeArm);

    osg::ref_ptr<TransparencyNode> fxNode = new TransparencyNode;
    fxNode->addChild(patUpperArm);
    root->addChild(fxNode);

    //root->addChild(createAxis());
    root->addChild(createMeshCube());

    osg::DisplaySettings::instance()->setNumMultiSamples(8);
    osgViewer::Viewer visor;
    visor.setSceneData(root);
    visor.setUpViewInWindow(0,0,1000,1000,0);
    osgViewer::Viewer::Windows windows;
    visor.getWindows(windows);
    windows[0]->setWindowName("3D Env");

    visor.setCameraManipulator(new osgGA::TrackballManipulator);
    visor.getCamera()->setClearColor(osg::Vec4(0.8f,0.8f,0.8f,1.0f));

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

    Eigen::Vector3f target(5.0, 0.5, -2.0);

    osg::ref_ptr<osg::ShapeDrawable> shape1 = new osg::ShapeDrawable;
    shape1->setShape( new osg::Box(osg::Vec3(target(0), target(1), target(2)), 0.5f) );
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
    Eigen::Vector3f currentWristPos;
    SolveArmInvKinematics(target, currentWristPos, shoulderQuat, elbowQuat);

    patBox->setPosition(osg::Vec3d(currentWristPos(0),currentWristPos(1),currentWristPos(2)));

    patUpperArm->setAttitude(osg::Quat(shoulderQuat.x(),shoulderQuat.y(),shoulderQuat.z(),shoulderQuat.w()));
    patForeArm->setAttitude(osg::Quat(elbowQuat.x(),elbowQuat.y(),elbowQuat.z(),elbowQuat.w()));

    context_t context(1);
    socket_t subscriber(context, ZMQ_REQ);
    subscriber.connect("ipc:///tmp/graphics.pipe");
    //subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::message_t state_msg;

    osg::Box* currentCube = new osg::Box(osg::Vec3(2.5,0.5,-2.0), 5, 0.5, 0.5);
    osg::ShapeDrawable* currentCubeDrawable = new osg::ShapeDrawable(currentCube);
    currentCubeDrawable->setColor(osg::Vec4(0.2,0.6,1.0,1.0));
    currentCubeDrawable->setUseDisplayList(false);
    osg::Geode* basicShapesGeode = new osg::Geode();
    basicShapesGeode->addDrawable(currentCubeDrawable);
    root->addChild(basicShapesGeode);

    osg::Box* nextCube = new osg::Box(osg::Vec3(2.5,0.5,-2.0), 5, 0.5, 0.5);
    osg::ShapeDrawable* nextCubeDrawable = new osg::ShapeDrawable(nextCube);
    nextCubeDrawable->setColor(osg::Vec4(0.2,0.6,1.0,1.0));
    nextCubeDrawable->setUseDisplayList(false);
    osg::Geode* nextShapesGeode = new osg::Geode();
    nextShapesGeode->addDrawable(nextCubeDrawable);
    root->addChild(nextShapesGeode);

    GuiKeyboardEventHandler* guiEventHandler = new GuiKeyboardEventHandler();
    visor.addEventHandler(guiEventHandler);

    osg::ref_ptr<osg::MatrixTransform> parent =
    new osg::MatrixTransform;
    parent->setMatrix( osg::Matrix::rotate(
    osg::PI_2, osg::Y_AXIS) * osg::Matrix::translate(
    8.0f,-10.0f,-3.0f) );

    osgParticle::ParticleSystem* fire = createFireParticles(parent.get() );

    osg::ref_ptr<osgParticle::ParticleSystemUpdater> updater =
    new osgParticle::ParticleSystemUpdater;
    updater->addParticleSystem(fire);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( fire );

    osg::ref_ptr<osg::Switch> fireSwitch = new osg::Switch;
    fireSwitch->addChild(parent.get(), false);
    fireSwitch->addChild( updater.get(), true );
    fireSwitch->addChild( geode.get(), true );

    root->addChild(fireSwitch);

    fireSwitch->

    while ( !visor.done()){

        cout<<"send"<<endl;
        if (pauseGame) {
            subscriber.send("c", 2);
        } else {
            subscriber.send("p", 2);
        }
        cout<<"sent"<<endl;
        subscriber.recv(&state_msg);
        cout<<"received"<<endl;
        string state_str(((char *)state_msg.data()));
        cout<<"message: "<<state_str<<endl;
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
            ss>>handPos[0];
            ss>>handPos[1];
            ss>>handPos[2];

            currentCube->setCenter(osg::Vec3(5.0 - (currentBlockLen)/2.0 * timeScale, 2.5 + currentBlockX, -2.0));
            currentCube->setHalfLengths(osg::Vec3((currentBlockLen)/2.0 * timeScale,0.25,0.25));
            nextCube->setCenter(osg::Vec3(5.0 - nextBlockStart * timeScale - (nextBlockLen)/2.0 * timeScale, 2.5 + nextBlockX, -2.0));
            nextCube->setHalfLengths(osg::Vec3((nextBlockLen)/2.0 * timeScale,0.25,0.25));

            target(0) = 5.0;
            target(1) = 2.5 + handPos[0];
            target(2) = -2.0;

            shoulderQuat = shoulderQuatO;
            elbowQuat = elbowQuatO;
            SolveArmInvKinematics(target, currentWristPos, shoulderQuat, elbowQuat);
            patUpperArm->setAttitude(osg::Quat(shoulderQuat.x(),shoulderQuat.y(),shoulderQuat.z(),shoulderQuat.w()));
            patForeArm->setAttitude(osg::Quat(elbowQuat.x(),elbowQuat.y(),elbowQuat.z(),elbowQuat.w()));

        }

        visor.frame();
    }

    return 0;
}

