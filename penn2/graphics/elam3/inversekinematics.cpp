#include "inversekinematics.h"

#include <iostream>
#include <chrono>

using namespace std;

// solves the inverse kinematics problem
// targetPos: the 3d position of the target position that hand/end-point should go there
// currentPos:
double SolveArmInvKinematics(Eigen::Vector3d targetPos, Eigen::Vector3d& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat,
                            osg::ref_ptr<osg::PositionAttitudeTransform> upperArm, osg::ref_ptr<osg::PositionAttitudeTransform> foreArm,
                            osg::ref_ptr<osg::Geode> handGeode, osg::ref_ptr<osg::Geode> elbowGeode) {
    double error = 1.0;

    // assume the angles are all zero and the initial positions are given
    // ge the delta angles and then add to the osg quats

    // set axises of the rotation joints
    Eigen::Vector3d elbowAxisZ(0.0, 0.0, 1.0);
    Eigen::Vector3d shoulderAxisX(1.0, 0.0, 0.0);
    Eigen::Vector3d shoulderAxisY(0.0, 1.0, 0.0);
    Eigen::Vector3d shoulderAxisZ(0.0, 0.0, 1.0);


    //
    /// find out what's the thing with the flipped sign in the elbow pos and wrist pos
//    Eigen::Vector3d handPos(1.50601, -0.23667, -0.30729);
//    Eigen::Vector3d shoulderPos(0.0, 0.0, -2.0);
//    Eigen::Vector3d elbowPos(-4.92282, -0.1, -2.0);
//    Eigen::Vector3d wristPos(-8.78673, -0.09678, 0.04826 - 2.0);

    Eigen::Vector3d shoulderPos(0.0, 0.0, -2.0); // accurate position
    Eigen::Vector3d elbowPos(4.92282, 0.0999968, -2.0); // accurate position
//    Eigen::Vector3d wristPos(8.72282, 0.206994, -1.96923); // accurate position
    Eigen::Vector3d wristPos(8.72282, 0.0999968, -2.0); // test

//    Eigen::Vector3d currentShoulderAxisX = shoulderQuat._transformVector(shoulderAxisX);
//    Eigen::Vector3d currentShoulderAxisY = shoulderQuat._transformVector(shoulderAxisY);
//    Eigen::Vector3d currentShoulderAxisZ = shoulderQuat._transformVector(shoulderAxisZ);
//    Eigen::Vector3d currentElbowAxisZ = shoulderQuat._transformVector(elbowAxisZ);

    static Eigen::Vector3d currentShoulderAxisX = shoulderAxisX;
    static Eigen::Vector3d currentShoulderAxisY = shoulderAxisY;
    static Eigen::Vector3d currentShoulderAxisZ = shoulderAxisZ;
    static Eigen::Vector3d currentElbowAxisZ = elbowAxisZ;

    static Eigen::Vector3d currentElbowPos = elbowPos;//shoulderQuat._transformVector(elbowPos);
    static Eigen::Vector3d currentWristPos = wristPos;//shoulderQuat._transformVector(wristPos);

    //currentWristPos -= currentElbowPos;
    //currentWristPos = elbowQuat._transformVector(currentWristPos);
    //currentWristPos += currentElbowPos;


    auto start = chrono::high_resolution_clock::now();
    // ik 2 iterate until error is small
    unsigned iter = 0;
    cout<<"before the loop"<<endl;
    while ((error > 0.01) && (iter<5000) ) {
        iter += 1;
//        cout<<"\n**** iter: "<<iter<<endl;

        // ik 3 get current orientations and positions

        // ik 4 compute jacobian
        Eigen::Vector3d jacobCol1 = currentShoulderAxisX.cross(currentWristPos - shoulderPos);
        Eigen::Vector3d jacobCol2 = currentShoulderAxisY.cross(currentWristPos - shoulderPos);
        Eigen::Vector3d jacobCol3 = currentShoulderAxisZ.cross(currentWristPos - shoulderPos);
        Eigen::Vector3d jacobCol4 = currentElbowAxisZ.cross(currentWristPos - currentElbowPos);

        Eigen::Matrix<double, 3, 4> jacobian;
        jacobian.col(0) = jacobCol1;
        jacobian.col(1) = jacobCol2;
        jacobian.col(2) = jacobCol3;
        jacobian.col(3) = jacobCol4;

        // target - currect position
        Eigen::Vector3d displacement = targetPos - currentWristPos;

        /// debug
//        cout<<"jacobian: "<<jacobian<<endl;
        ///


        Eigen::Vector3d jjtd = jacobian * jacobian.transpose() * displacement;

        double alpha = displacement.dot(jjtd) / jjtd.dot(jjtd) * 0.5;
        //cout<<"alpha "<<alpha<<endl;

        // ik 5 compute rotation updates
        Eigen::Vector4d deltaTheta = alpha * jacobian.transpose() * displacement;

        ///debug
        //cout<<"deltaTheta: "<<deltaTheta<<endl;
        ///


        //cout<<"deltaTheta "<<deltaTheta<<endl;

        // ik 5 update orientations
        Eigen::Quaternion<double> deltaShoulderQuat =
                Eigen::AngleAxis<double>(deltaTheta(0), currentShoulderAxisX) *
                Eigen::AngleAxis<double>(deltaTheta(1), currentShoulderAxisY) *
                Eigen::AngleAxis<double>(deltaTheta(2), currentShoulderAxisZ);
        shoulderQuat = deltaShoulderQuat.cast<float>() * shoulderQuat;

        Eigen::Quaternion<double> deltaElbowQuat (Eigen::AngleAxis<double>(deltaTheta(3), currentElbowAxisZ));
        elbowQuat = deltaElbowQuat.cast<float>() * elbowQuat;

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

        Eigen::Vector3d oldElbowPos = currentElbowPos;

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

        Eigen::Vector3d oldWristPos = currentWristPos;

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
//        cout<<"targetPos: "<<targetPos<<endl;
//        cout<<"currentWristPos: "<<currentWristPos<<endl;
//        cout<<"currentElbowPos: "<<currentElbowPos<<endl;
//        //cout<<"currentElbowPos - currentWristPos: "<<currentElbo<<endl;
//        cout<<"displacement: "<<displacement<<endl;
//        cout<<"currentElbowAxisZ: "<<currentElbowAxisZ<<endl;
//        cout<<"currentShoulderAxisX: "<<currentShoulderAxisX<<endl;
//        cout<<"currentShoulderAxisY: "<<currentShoulderAxisY<<endl;
//        cout<<"currentShoulderAxisZ: "<<currentShoulderAxisZ<<endl;
        ///

        error = displacement.norm();
//        cout<<"error: "<<error<<endl;


        //this_thread::sleep_for(chrono::milliseconds(500));

    }
    cout<<"after the loop"<<endl;
    auto end = chrono::high_resolution_clock::now();
    cout<<chrono::duration_cast<chrono::nanoseconds>(end-start).count()<<"ns\n";

    currentPos = currentWristPos;

    return error;
}
