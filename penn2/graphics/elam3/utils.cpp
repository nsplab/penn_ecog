
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
