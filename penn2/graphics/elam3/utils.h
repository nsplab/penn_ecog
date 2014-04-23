#ifndef UTILS_H
#define UTILS_H

#include <osg/io_utils>
#include <osgGA/TrackballManipulator>
#include <osg/Camera>
#include <osgText/Text>
#include <osgText/Font>

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
#endif // UTILS_H
