#ifndef INVERSEKINEMATICS_H
#define INVERSEKINEMATICS_H

#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

double SolveArmInvKinematics(Eigen::Vector3d targetPos, Eigen::Vector3d& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat,
                            osg::ref_ptr<osg::PositionAttitudeTransform> upperArm, osg::ref_ptr<osg::PositionAttitudeTransform> foreArm,
                            osg::ref_ptr<osg::Geode> handGeode, osg::ref_ptr<osg::Geode> elbowGeode);

#endif // INVERSEKINEMATICS_H
