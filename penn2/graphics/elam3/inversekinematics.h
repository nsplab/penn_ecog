#ifndef INVERSEKINEMATICS_H
#define INVERSEKINEMATICS_H

#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>


class InverseKinematics
{
public:
    InverseKinematics();
    float SolveArmInvKinematics(Eigen::Vector3f targetPos, Eigen::Vector3f& currentPos, Eigen::Quaternionf& shoulderQuat, Eigen::Quaternionf& elbowQuat, osg::ref_ptr<osg::PositionAttitudeTransform> patUpperArm);
private:
    osg::ref_ptr<osg::PositionAttitudeTransform> patUpperArm;
    osg::ref_ptr<osg::PositionAttitudeTransform> patForeArm;
    osg::ref_ptr<osg::ShapeDrawable> elbowPoint;
    osg::ref_ptr<osg::Geode> elbowPointGeode;
    osg::ref_ptr<osg::ShapeDrawable> shoulderPoint;
    osg::ref_ptr<osg::Geode> shoulderPointGeode;
};

#endif // INVERSEKINEMATICS_H
