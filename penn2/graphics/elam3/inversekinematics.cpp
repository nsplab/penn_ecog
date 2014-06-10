#include "inversekinematics.h"

#include <iostream>
#include <chrono>

using namespace std;

// solves the inverse kinematics problem
// targetPos: the 3d position of the target position that hand/end-point should go there
// currentPos:
float SolveArmInvKinematics(Eigen::Vector3f targetPos, Eigen::Vector3f& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat,
                            osg::ref_ptr<osg::PositionAttitudeTransform> upperArm, osg::ref_ptr<osg::PositionAttitudeTransform> foreArm,
                            osg::ref_ptr<osg::Geode> handGeode, osg::ref_ptr<osg::Geode> elbowGeode) {
    float error = 1.0;

    // assume the angles are all zero and the initial positions are given
    // ge the delta angles and then add to the osg quats

    // set axises of the rotation joints
    Eigen::Vector3f elbowAxisZ(0.0, 0.0, 1.0);
    Eigen::Vector3f shoulderAxisX(1.0, 0.0, 0.0);
    Eigen::Vector3f shoulderAxisY(0.0, 1.0, 0.0);
    Eigen::Vector3f shoulderAxisZ(0.0, 0.0, 1.0);


    //
    /// find out what's the thing with the flipped sign in the elbow pos and wrist pos
//    Eigen::Vector3f handPos(1.50601, -0.23667, -0.30729);
//    Eigen::Vector3f shoulderPos(0.0, 0.0, -2.0);
//    Eigen::Vector3f elbowPos(-4.92282, -0.1, -2.0);
//    Eigen::Vector3f wristPos(-8.78673, -0.09678, 0.04826 - 2.0);

    Eigen::Vector3f shoulderPos(0.0, 0.0, -2.0); // accurate position
    Eigen::Vector3f elbowPos(4.92282, 0.0999968, -2.0); // accurate position
//    Eigen::Vector3f wristPos(8.72282, 0.206994, -1.96923); // accurate position
    Eigen::Vector3f wristPos(8.72282, 0.0999968, -2.0); // test

//    Eigen::Vector3f currentShoulderAxisX = shoulderQuat._transformVector(shoulderAxisX);
//    Eigen::Vector3f currentShoulderAxisY = shoulderQuat._transformVector(shoulderAxisY);
//    Eigen::Vector3f currentShoulderAxisZ = shoulderQuat._transformVector(shoulderAxisZ);
//    Eigen::Vector3f currentElbowAxisZ = shoulderQuat._transformVector(elbowAxisZ);

    Eigen::Vector3f currentShoulderAxisX = shoulderAxisX;
    Eigen::Vector3f currentShoulderAxisY = shoulderAxisY;
    Eigen::Vector3f currentShoulderAxisZ = shoulderAxisZ;
    Eigen::Vector3f currentElbowAxisZ = elbowAxisZ;

    Eigen::Vector3f currentElbowPos = elbowPos;//shoulderQuat._transformVector(elbowPos);
    Eigen::Vector3f currentWristPos = wristPos;//shoulderQuat._transformVector(wristPos);

    //currentWristPos -= currentElbowPos;
    //currentWristPos = elbowQuat._transformVector(currentWristPos);
    //currentWristPos += currentElbowPos;


    auto start = chrono::high_resolution_clock::now();
    // ik 2 iterate until error is small
    unsigned iter = 0;
    while ((error > 0.001) ) {
        iter += 1;
        cout<<"\n**** iter: "<<iter<<endl;

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

        /// debug
        cout<<"displacement: "<<displacement<<endl;
        ///


        Eigen::Vector3f jjtd = jacobian * jacobian.transpose() * displacement;

        float alpha = displacement.dot(jjtd) / jjtd.dot(jjtd) * 0.5;
        //cout<<"alpha "<<alpha<<endl;

        // ik 5 compute rotation updates
        Eigen::Vector4f deltaTheta = alpha * jacobian.transpose() * displacement;

        ///debug
        cout<<"deltaTheta: "<<deltaTheta<<endl;
        ///


        //cout<<"deltaTheta "<<deltaTheta<<endl;

        // ik 5 update orientations
        Eigen::Quaternion<float> deltaShoulderQuat =
                Eigen::AngleAxis<float>(deltaTheta(0), currentShoulderAxisX) *
                Eigen::AngleAxis<float>(deltaTheta(1), currentShoulderAxisY) *
                Eigen::AngleAxis<float>(deltaTheta(2), currentShoulderAxisZ);
        shoulderQuat = deltaShoulderQuat * shoulderQuat;

        Eigen::Quaternion<float> deltaElbowQuat (Eigen::AngleAxis<float>(deltaTheta(3), currentElbowAxisZ));
        elbowQuat = deltaElbowQuat * elbowQuat;

        currentShoulderAxisX = deltaShoulderQuat._transformVector(currentShoulderAxisX);
        currentShoulderAxisY = deltaShoulderQuat._transformVector(currentShoulderAxisY);
        currentShoulderAxisZ = deltaShoulderQuat._transformVector(currentShoulderAxisZ);
        currentElbowAxisZ = deltaShoulderQuat._transformVector(currentElbowAxisZ);
        //cout<<"currentElbowAxisZ "<<currentElbowAxisZ<<endl;
        //cout<<"elbowQuat "<<elbowQuat.w()<<" "<<elbowQuat.x()<<" "<<elbowQuat.y()<<" "<<elbowQuat.z()<<endl;

        upperArm->setAttitude(osg::Quat(shoulderQuat.x(),shoulderQuat.y(),shoulderQuat.z(),shoulderQuat.w()));
        foreArm->setAttitude(osg::Quat(elbowQuat.x(),elbowQuat.y(),elbowQuat.z(),elbowQuat.w()));

        osg::Vec3d currentWristPosOsg = handGeode->getBoundingBox().center() * handGeode->getWorldMatrices()[0];
        osg::Vec3d currentElbowPosOsg = elbowGeode->getBoundingBox().center() * elbowGeode->getWorldMatrices()[0];
        for (unsigned t=0; t<3; t++) {
            currentWristPos[t] = currentWristPosOsg[t];
            currentElbowPos[t] = currentElbowPosOsg[t];
        }

        // compute the current position of the elbow given the rotation of the shoulder
        // 1. move the shoulder to the origin, i.e., move the elbow according to that translation
        // 2. rotate the elbow joint position according to the shoulder rotation
        // 3. move back the shoulder to  its world coordinate, i.e, move the elbow according to that translation

        if (false) {

        Eigen::Vector3f oldElbowPos = currentElbowPos;

        currentElbowPos = currentElbowPos - shoulderPos;
        currentElbowPos = deltaShoulderQuat._transformVector(currentElbowPos);
        currentElbowPos = currentElbowPos + shoulderPos;


        // 7. move the elbow to the origin, i.e., move the wrist/hand according to that translation
        // 8. rotate the  position wrist/hand according to the elbow rotation
        // 9. move back the elbow to  its world coordinate, i.e, move the wrist/hand according to that translation

        currentWristPos = currentWristPos - oldElbowPos;
        currentWristPos = deltaElbowQuat._transformVector(currentWristPos);
        currentWristPos = currentWristPos + currentElbowPos;

        // 4. move the shoulder to the origin, i.e., move the wrist/hand according to that translation
        // 5. rotate the  position wrist/hand according to the shoulder rotation
        // 6. move back the shoulder to  its world coordinate, i.e, move the wrist/hand according to that translation

        Eigen::Vector3f oldWristPos = currentWristPos;

        currentWristPos = oldWristPos - shoulderPos;
//        currentWristPos = currentWristPos - shoulderPos;
        currentWristPos = deltaShoulderQuat._transformVector(currentWristPos);
        currentWristPos = currentWristPos + shoulderPos;

        // 7. move the elbow to the origin, i.e., move the wrist/hand according to that translation
        // 8. rotate the  position wrist/hand according to the elbow rotation
        // 9. move back the elbow to  its world coordinate, i.e, move the wrist/hand according to that translation

//      currentWristPos = currentWristPos - oldElbowPos;
//        currentWristPos = currentWristPos - currentElbowPos;
//        currentWristPos = deltaElbowQuat._transformVector(currentWristPos);
//        currentWristPos = currentWristPos + currentElbowPos;
        }


        displacement = targetPos - currentWristPos;

        ///debug
        cout<<"targetPos: "<<targetPos<<endl;
        cout<<"currentWristPos: "<<currentWristPos<<endl;
        cout<<"currentElbowPos: "<<currentElbowPos<<endl;
        //cout<<"currentElbowPos - currentWristPos: "<<currentElbo<<endl;
        cout<<"displacement: "<<displacement<<endl;
        cout<<"currentElbowAxisZ: "<<currentElbowAxisZ<<endl;
        cout<<"currentShoulderAxisX: "<<currentShoulderAxisX<<endl;
        cout<<"currentShoulderAxisY: "<<currentShoulderAxisY<<endl;
        cout<<"currentShoulderAxisZ: "<<currentShoulderAxisZ<<endl;
        ///

        error = displacement.norm();
        cout<<"error: "<<error<<endl;


        //this_thread::sleep_for(chrono::milliseconds(500));

    }
    auto end = chrono::high_resolution_clock::now();
    cout<<chrono::duration_cast<chrono::nanoseconds>(end-start).count()<<"ns\n";

    currentPos = currentWristPos;

    return error;
}
