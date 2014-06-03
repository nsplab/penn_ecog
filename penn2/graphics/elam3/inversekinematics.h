#ifndef INVERSEKINEMATICS_H
#define INVERSEKINEMATICS_H

#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

float SolveArmInvKinematics(Eigen::Vector3f targetPos, Eigen::Vector3f& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat,
                            osg::ref_ptr<osg::PositionAttitudeTransform> upperArm, osg::ref_ptr<osg::PositionAttitudeTransform> foreArm,
                            osg::ref_ptr<osg::Geode> handGeode, osg::ref_ptr<osg::Geode> elbowGeode);

#endif // INVERSEKINEMATICS_H
