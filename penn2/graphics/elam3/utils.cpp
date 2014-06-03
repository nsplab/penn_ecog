
#include "utils.h"

bool pauseGame = false;

bool GuiKeyboardEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
    case(osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON):
    {
        break;
    }
    case(osgGA::GUIEventAdapter::KEYDOWN):
    {
        switch(ea.getKey())
        {
        case 'l':
            return false;
            break;

        case osgGA::GUIEventAdapter::KEY_Space:
            pauseGame = !pauseGame;
            return true;

        default:
            return false;
        }
    }
    default:
        return false;
    }
}


osg::Camera* createHUDCamera( double left, double right,
                              double bottom, double top ) {
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix(
    osg::Matrix::ortho2D(left, right, bottom, top) );
    return camera.release();
}

osgText::Text* createText( const osg::Vec3& pos, const std::string& content,
                            float size, osg::ref_ptr<osgText::Font> g_font ) {
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont( g_font.get() );
    text->setCharacterSize( size );
    text->setFontResolution(100,100);
    text->setAxisAlignment( osgText::TextBase::XY_PLANE );
    text->setPosition( pos );
    text->setText( content );
    text->setColor(osg::Vec4(1.0, 1.0, 0.0, 1.0));
    text->setBackdropType(osgText::Text::OUTLINE);
    text->setBackdropColor(osg::Vec4(0.0,0.0,0.0,1.0));
    return text.release();
}

osgText::Text* createText2( const osg::Vec3& pos, const std::string& content,
                            float size, osg::ref_ptr<osgText::Font> g_font ) {
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont( g_font.get() );
    text->setCharacterSize( size );
    text->setFontResolution(100,100);
    text->setAxisAlignment( osgText::TextBase::XY_PLANE );
    text->setPosition( pos );
    text->setText( content );
    text->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
    text->setBoundingBoxColor(osg::Vec4(0.9, 0.9, 0.9, 1.0));
    return text.release();
}

//Make borders around plot in top left that keeps track of performance
osg::Geode* createPlotDecoration(float scaleX, float scaleY) {
    osg::Vec3Array* vertices = new osg::Vec3Array();

    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec4Array* colors   = new osg::Vec4Array();

    vertices->push_back(osg::Vec3(0, 700-scaleY, 0.0f));
    vertices->push_back(osg::Vec3(scaleX, 700-scaleY, 0.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

    vertices->push_back(osg::Vec3(0, 700, 0.0f));
    vertices->push_back(osg::Vec3(scaleX, 700, 0.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

    vertices->push_back(osg::Vec3(scaleX, 700, 0.0f));
    vertices->push_back(osg::Vec3(scaleX, 700-scaleY, 0.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

    vertices->push_back(osg::Vec3(0, 700, 0.0f));
    vertices->push_back(osg::Vec3(0, 700-scaleY, 0.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

    vertices->push_back(osg::Vec3(0, 700-scaleY/2.0, 0.0f));
    vertices->push_back(osg::Vec3(scaleX, 700-scaleY/2.0, 0.0f));
    colors->push_back(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    colors->push_back(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));

    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertices->size()));
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, false);

    geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    geometry->getOrCreateStateSet()->setRenderBinDetails(13, "RenderBin");

    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(2.0f);

    geode->addDrawable(geometry);

    osg::Box* backBox = new osg::Box(osg::Vec3(scaleX/2.0,700-scaleY/2.0,0), scaleX+20, scaleY+20, 0);
    osg::ShapeDrawable* backBoxDrawable = new osg::ShapeDrawable(backBox);
    backBoxDrawable->setColor(osg::Vec4(0.9,0.9,0.9,1.0));
    backBoxDrawable->getOrCreateStateSet()->setRenderBinDetails(13, "RenderBin");

    geode->addDrawable(backBoxDrawable);

    geode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);


    return geode;
}

//Function to plot performance in the top left corner of the screen
osg::Geode* createPlot(float scaleX, float scaleY, unsigned numberOfPoints, osg::Vec3Array** plotArray) {
    (*plotArray) = new osg::Vec3Array();

    osg::Geode*     geode    = new osg::Geode();
    osg::Geometry*  geometry = new osg::Geometry();
    osg::Vec4Array* colors   = new osg::Vec4Array();

    for (unsigned i=0; i<=numberOfPoints; i++) {
        (*plotArray)->push_back(osg::Vec3(float(i)/numberOfPoints * scaleX, 700-scaleY, 0.0f));
        //(*plotArray)->push_back(osg::Vec3(float(i+1)/numberOfPoints * scaleX, 700-scaleY, 0.0f));
        colors->push_back(osg::Vec4(0.2f, 0.2f, 0.7f, 1.0f));
        //colors->push_back(osg::Vec4(0.3f, 0.0f, 0.0f, 1.0f));
    }

    geometry->setVertexArray((*plotArray));
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, (*plotArray)->size()));
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    geometry->setUseDisplayList(false);

    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(2.0f);

    geode->addDrawable(geometry);

    geode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);

    return geode;
}

//Implement smoke particles when wrist intersects with blue bars
osgParticle::ParticleSystem* createFireParticles(
        osg::Group* parent ) {
    osg::ref_ptr<osgParticle::ParticleSystem> ps =
    new osgParticle::ParticleSystem;
    ps->getDefaultParticleTemplate().setLifeTime( 1.5f );
    ps->getDefaultParticleTemplate().setShape(
    osgParticle::Particle::QUAD );
    ps->getDefaultParticleTemplate().setSizeRange(
    osgParticle::rangef(0.5f, 0.05f) );
    ps->getDefaultParticleTemplate().setAlphaRange(
    osgParticle::rangef(0.6f, 0.1f) );
    ps->getDefaultParticleTemplate().setColorRange(
    osgParticle::rangev4(osg::Vec4(0.1f,0.3f,0.4f,1.0f),
    osg::Vec4(0.0f,0.1f,0.3f,0.5f)) );
    ps->setDefaultAttributes("../smoke.rgb", true, false );

    osg::ref_ptr<osgParticle::RandomRateCounter> rrc =
    new osgParticle::RandomRateCounter;
    rrc->setRateRange( 5, 20 );
    osg::ref_ptr<osgParticle::RadialShooter> shooter =
    new osgParticle::RadialShooter;
    shooter->setThetaRange( -osg::PI_4/3.0, osg::PI_4/3.0 );
    shooter->setPhiRange( -osg::PI_4/3.0, osg::PI_4/3.0 );
    shooter->setInitialSpeedRange( 1.0f, 6.5f );

    osg::ref_ptr<osgParticle::ModularEmitter> emitter =
    new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );
    emitter->setCounter( rrc.get() );
    emitter->setShooter( shooter.get() );
    parent->addChild( emitter.get() );
    return ps.get();
}

//create an object that can be rendered on the screen to display the joint axes
//used for debugging, not called during regular operation
osg::Geode* createAxis(float scale) {
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

//create the grid line objects that define the walls of the 3d virtual environment
//used only for visualization; does not actually interact with the virtual coordinates of the arm
//there is no collision detection between the arm and these walls

osg::Geode* createMeshCube(float scale, float depth)
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

    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(2.0f);

    geode->addDrawable(geometry);

    geode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);


    return geode;
}
