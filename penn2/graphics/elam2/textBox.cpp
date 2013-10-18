#include "textBox.h"
////////////////////////////////////////////////////////////////////////////////
TextBox::TextBox(unsigned int id):
  text(new osgText::Text()),
  textString("Default Text"),
  id(id) {

  text->setText(textString);

  //Set the screen alignment - always face the screen
  text->setAxisAlignment(osgText::Text::SCREEN);
}

void TextBox::setText(const string& t){
  textString = t;
  text->setText(textString);
}

osg::Vec2d TextBox::getPosition() const{
  // Grab the Vec3 object and make/return a new Vec2 object since we're only
  // going to be using the text box for the HUD at the moment. This may change
  // if we decide to use the text box in the scene graph as tags for 3D elements
  const osg::Vec3f& current_position = text->getPosition();
  return osg::Vec2d(current_position.x(), current_position.y());
}
