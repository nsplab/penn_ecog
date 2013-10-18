#ifndef UTILS_H
#define UTILS_H


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

#define GRASP_STEPS 10

osg::Geode* createAxis();

struct BoneFinder : public osg::NodeVisitor
{
    std::vector<osgAnimation::Bone*> _bones;
    BoneFinder() :     osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    void apply(osg::Transform& node) {
        osgAnimation::Bone* bone =   dynamic_cast<osgAnimation::Bone*>(&node);
        if (bone){
            _bones.push_back(bone);
        }
    traverse(node);
    }
};

void themeMusic();
void tadaMusic();
void dingMusic();
int PowerToPixel(float mean_power);

/*void solveIK(osg::Vec3 endpoint, osg::Vec3 currentpoint, osg::Vec3 jointpos, double& angle, Eigen::Vector3d rot_axis, double alpha=0.005){
    // q a vector of all angles/thetas
    // x the tip of manipulator 3-vector
    // x = f(q)
    //

    // ds_i / dtheta_j = v_j X (s_j - p_j)

    osg::Vec3f distance = currentpoint - jointpos;
    //cout<<"d: "<<distance<<endl;

    osg::Vec3f jacob1 = osg::Vec3f(rot_axis(0), rot_axis(1), rot_axis(2)).operator ^(distance);

    osg::Vec3 error = currentpoint - endpoint;

    angle = -alpha * (jacob1 * error);

        if (angle > 0.1)
            angle = 0.1;
        else if (angle < -0.1)
            angle = -0.1;

    //cout<<"delta: "<<angles[0]<<endl;
}*/

osg::Camera* createHUDCamera( double left, double right,
                              double bottom, double top );

osgText::Text* createText( const osg::Vec3& pos,
                           const std::string& content,float size, bool cfont = false,
                           osg::Vec4 tcolor = osg::Vec4(1.0,1.0,1.0,1.0));

void graspObj(bool grasp, BoneFinder& boneFinder, int step);

class GuiKeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    GuiKeyboardEventHandler(bool demoMode_):GUIEventHandler()
    {step = GRASP_STEPS;handState = true;demoMode = demoMode_;}
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    virtual void accept(osgGA::GUIEventHandlerVisitor& v)   { v.visit(*this); };
    void iterate() {if (step < GRASP_STEPS) step += 1; graspObj(handState, *(boneFinder), step);};
    BoneFinder* boneFinder;
    int step;
    bool handState;
    bool demoMode;
};


#endif // UTILS_H
