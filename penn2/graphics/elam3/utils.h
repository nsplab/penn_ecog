#ifndef UTILS_H
#define UTILS_H

#include <osg/io_utils>
#include <osgGA/TrackballManipulator>
#include <osg/Camera>
#include <osgText/Text>
#include <osgText/Font>

#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osg/PointSprite>
#include <osg/Point>

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
#include <osgFX/Cartoon>

#include <osgText/Font>
#include <osgText/Text>

//tmp
#include <osg/ShapeDrawable>


class GuiKeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    GuiKeyboardEventHandler():GUIEventHandler()
    {}
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
};

osg::Camera* createHUDCamera( double left, double right,
                              double bottom, double top );

osgText::Text* createText( const osg::Vec3& pos, const std::string& content,
                            float size, osg::ref_ptr<osgText::Font> g_font);
osgText::Text* createText2( const osg::Vec3& pos, const std::string& content,
                            float size, osg::ref_ptr<osgText::Font> g_font);


//Implement smoke particles when wrist intersects with blue bars
osgParticle::ParticleSystem* createFireParticles(
        osg::Group* parent );

//Make borders around plot in top left that keeps track of performance
osg::Geode* createPlotDecoration(float scaleX, float scaleY);

//Function to plot performance in the top left corner of the screen
osg::Geode* createPlot(float scaleX, float scaleY, unsigned numberOfPoints, osg::Vec3Array** plotArray);

//create an object that can be rendered on the screen to display the joint axes
//used for debugging, not called during regular operation
osg::Geode* createAxis(float scale = 10.0);

//create the grid line objects that define the walls of the 3d virtual environment
//used only for visualization; does not actually interact with the virtual coordinates of the arm
//there is no collision detection between the arm and these walls

osg::Geode* createMeshCube(float scale = 5.0f, float depth = 5.0f);

//This class is used to implement semi-transparent objects in the graphics
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

//This class is used to implement semi-transparent objects in the graphics; more realistic than built-in OpenGL transparency
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

//This class is used to implement semi-transparent objects in the graphics; more realistic than built-in OpenGL transparency
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

#endif // UTILS_H
