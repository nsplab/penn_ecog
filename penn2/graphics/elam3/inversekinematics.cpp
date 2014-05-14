#include "inversekinematics.h"

#include <iostream>
#include <chrono>

using namespace std;

InverseKinematics::InverseKinematics()
{
    patUpperArm = new osg::PositionAttitudeTransform();
    patUpperArm->setPosition(osg::Vec3(0,0,-2.0));
    patUpperArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(3.141592, osg::Vec3d(0,0,1)));

    patForeArm = new osg::PositionAttitudeTransform();
    patForeArm->setPosition(osg::Vec3(0,0,-2.0));
    patForeArm->setAttitude(osg::Quat(0, osg::Vec3d(1,0,0)) * osg::Quat(0, osg::Vec3d(0,1,0)) * osg::Quat(0, osg::Vec3d(0,0,1)));


    elbowPoint = new osg::ShapeDrawable;
    elbowPoint->setShape( new osg::Sphere(osg::Vec3(0.0, 0.0, 0.0), 0.01f) );
    elbowPointGeode = new osg::Geode;
    elbowPointGeode->addDrawable(elbowPoint.get());
    patForeArm->addChild(elbowPointGeode);

    shoulderPoint = new osg::ShapeDrawable;
    shoulderPoint->setShape( new osg::Sphere(osg::Vec3(0.0, 0.0, 0.0), 0.01f) );
    shoulderPointGeode = new osg::Geode;
    shoulderPointGeode->addDrawable(shoulderPoint.get());
    patUpperArm->addChild(shoulderPointGeode);

}

float InverseKinematics::SolveArmInvKinematics(Eigen::Vector3f targetPos, Eigen::Vector3f& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat, osg::ref_ptr<osg::PositionAttitudeTransform> patUpperArm) {
    float error = 1.0;

    osg::ref_ptr<osg::PositionAttitudeTransform> tmpUpperArm = static_cast<osg::PositionAttitudeTransform*>(patUpperArm->cloneType());

    Eigen::Vector3f elbowAxisZ(0.0, 0.0, 1.0);
    Eigen::Vector3f shoulderAxisX(1.0, 0.0, 0.0);
    Eigen::Vector3f shoulderAxisY(0.0, 1.0, 0.0);
    Eigen::Vector3f shoulderAxisZ(0.0, 0.0, 1.0);

    Eigen::Vector3f handPos(1.50601, -0.23667, -0.30729);
    Eigen::Vector3f shoulderPos(0.0, 0.0, -2.0);
    Eigen::Vector3f elbowPos(4.92282, 0.0999968, -2.0);
    Eigen::Vector3f wristPos(8.72282, 0.206994, -1.96923);

    Eigen::Vector3f currentShoulderAxisX = shoulderQuat._transformVector(shoulderAxisX);
    Eigen::Vector3f currentShoulderAxisY = shoulderQuat._transformVector(shoulderAxisY);
    Eigen::Vector3f currentShoulderAxisZ = shoulderQuat._transformVector(shoulderAxisZ);
    Eigen::Vector3f currentElbowAxisZ = shoulderQuat._transformVector(elbowAxisZ);

    Eigen::Vector3f currentElbowPos = shoulderQuat._transformVector(elbowPos);
    Eigen::Vector3f currentWristPos = shoulderQuat._transformVector(wristPos);
    currentWristPos -= currentElbowPos;
    currentWristPos = elbowQuat._transformVector(currentWristPos);
    currentWristPos += currentElbowPos;

    cout<<"currentElbowPos: "<<endl;
    cout<<currentElbowPos<<endl;
    cout<<"currentWristPos: "<<endl;
    cout<<currentWristPos<<endl;



    auto start = chrono::high_resolution_clock::now();
    // ik 2 iterate until error is small
    while (error > 0.001) {

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

        float alpha = displacement.dot(jjtd) / jjtd.dot(jjtd) * 0.001;
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

        tmpUpperArm->setAttitude(osg::Quat(shoulderQuat.x(),shoulderQuat.y(),shoulderQuat.z(),shoulderQuat.w()));
        //cout<<"num children:  "<<tmpUpperArm->getNumChildren()<<endl;
        //osg::ref_ptr<osg::PositionAttitudeTransform> tmpForearm = static_cast<osg::PositionAttitudeTransform*>(tmpUpperArm->getChild(2));

        //cout<<"error "<<error<<endl;
        //this_thread::sleep_for(chrono::milliseconds(500));

    }
    auto end = chrono::high_resolution_clock::now();
    //cout<<chrono::duration_cast<chrono::nanoseconds>(end-start).count()<<"ns\n";

    currentPos = currentWristPos;

    return error;
}
