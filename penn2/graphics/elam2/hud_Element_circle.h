//###########################################################################
//  File:       hud_element.h
//  Purpose:    This class represents an individual HUD Element which represents
//              a 2D rectangle painted on the front of the screen, on top of
//              all other SceneData. The HUD Element may also contain Text Box
//              elements. Any Text Box element attached to this HUD Element
//              will automatically be cleaned up upon class destruction.
//############################################################################

#ifndef HUD_ELEMENT_CIRCLE_H
#define HUD_ELEMENT_CIRCLE_H
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Projection> 
#include <osgUtil/RenderBin>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include "textBox.h"


class HUD_Element_Circle{
public:
  // Constructs a default HUD Element of 100x100 pixels in
  // the lower lefthand corner with colors: (1.0, 0.5, 0.3, 0.5)
  HUD_Element_Circle(float x, float y, float r);
    
  ~HUD_Element_Circle();
	
  // Returns the drawable geometry of this HUD Element
  osg::Geode* getGeode();

  void setElementPosition   (float x, float y);
  void setElementColor      (float r, float g, float b, float a);
  void setElementAlpha      (float a);

  const osg::Vec2d&  getElementPosition() const;
  const float  getElementSize()     const;
  const osg::Vec4d&  getElementColor()    const;

  void addTextBox(TextBox& t);

  TextBox* getTextBox(unsigned int id)   const;
  osg::ref_ptr<osg::StateSet>         HE_StateSet;
private:

  osg::Vec2d       elementPosition;  //Lower left corner index position
  float       elementSize;      //radius of the element
  osg::Vec4d       elementColor;     //Color of the element
	
  osg::ref_ptr<osg::Geode>            HE_Geode;
  osg::ref_ptr<osg::Geometry>         HE_Geometry;
  osg::ref_ptr<osg::Vec3Array>        HE_Vertices;
  osg::ref_ptr<osg::DrawElementsUInt> HE_Indices;
  osg::ref_ptr<osg::Vec4Array>        HE_Color;
  osg::ref_ptr<osg::Vec3Array>        HE_Normals;
    
  vector<TextBox*> textBoxes;
};
#endif
