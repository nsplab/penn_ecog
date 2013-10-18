#ifndef TEXT_BOX_H
#define TEXT_BOX_H
#include <string>
using std::string;
#include <osgText/Text>

// This version of the text box will not be self sufficient in that it doesn't
// have the necessary objects to draw itself. It is simply a data structure
// that contains an osgText object and an id. It must be used with an external
// structure that contains the necessary objects to draw this osgText object.
// In this tutorial, the required object is the HUD Element and HUD.  

class TextBox {
public:
  TextBox(unsigned int id);
  ~TextBox () { }

  void setText(const string& text);
  void setFont(const string& font) { text->setFont(font); }
  void setColor(osg::Vec4d color) { text->setColor(color); }
  void setPosition(osg::Vec3d position) { text->setPosition(position);}
  void setTextSize(unsigned int size) { text->setCharacterSize(size); }
    
  osgText::Text& getTextObject() const { return *text; }
  unsigned int getId() const { return id; }
  const string& getText() const { return textString; }
  const osg::Vec4& getColor() const { return text->getColor(); }
  osg::Vec2d getPosition() const;
    
private:

  //Text object that will be drawn to screen
  osgText::Text *                     text;
    
  //String containing the text
  string                              textString;
    
  //The hopefully unique identifier for this text box
  unsigned int                        id;
};
#endif
