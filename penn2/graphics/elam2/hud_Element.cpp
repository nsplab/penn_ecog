#include "hud_Element.h"

HUD_Element::HUD_Element()
  : elementPosition(0.0, 0.0), //Default position is lower left corner of screen
  elementSize(100.0, 100.0),  //Make a square area of 100x100 pixels
  elementColor(1.0, 0.5, 0.3, 0.5),    //Set the default color
  HE_Geode(new osg::Geode()),
  HE_Geometry(new osg::Geometry()),
  HE_Vertices(new osg::Vec3Array()),

  //Define our shape to be a polygon
  HE_Indices(new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0)),
  HE_Color(new osg::Vec4Array()),
  HE_Normals(new osg::Vec3Array()),
  HE_StateSet(new osg::StateSet())
{
    HE_Geode->setDataVariance(osg::Object::DYNAMIC);
    HE_Vertices->setDataVariance(osg::Object::DYNAMIC);
    HE_Geometry->setDataVariance(osg::Object::DYNAMIC);
  // Construct the Element Geometry
  // Construct the vertices array by defining the 4 corners of the HUD Element
  //Lower-left vertex (x, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          elementPosition.y(), 
          0
      ) 
  );
  //Lower-right vertex (x + length, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          elementPosition.y(), 
          0
      ) 
  );
  //Upper-right vertex (x + length, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          (elementPosition.y() + elementSize.y()), 
          0
      ) 
  );
  //Upper-left vertex (x, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          (elementPosition.y() + elementSize.y()), 
          0
      ) 
  );

 // Set the indices - must be done in the order defined above.
  HE_Indices->push_back(0);
  HE_Indices->push_back(1);
  HE_Indices->push_back(2);
  HE_Indices->push_back(3);
	
  HE_Color->push_back(elementColor);
	
  // Set the normal in the positive z direction (torwards the user)
  // This is done to ensure that lighting falls on the front face of the hud
  // element polygon, and not the back face which we do not care about
  HE_Normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));

  //Set the geometry options
  HE_Geometry->setNormalArray  (HE_Normals.get());
  HE_Geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
  HE_Geometry->addPrimitiveSet (HE_Indices.get());
  HE_Geometry->setVertexArray  (HE_Vertices.get());
  HE_Geometry->setColorArray   (HE_Color.get());
  HE_Geometry->setColorBinding (osg::Geometry::BIND_OVERALL);
	
  //Add the geometry to the geode
  HE_Geode->addDrawable(HE_Geometry.get());
   
//////////////////////////////////////////////
// Setting State
//////////////////////////////////////////////
  // Set the OpenGL States of this Geometry Node
  HE_Geode->setStateSet(HE_StateSet.get());
  HE_StateSet->setRenderBinDetails(11, "RenderBin");
	
  // Says this geometry should blend with any geometry behind it if any
  // transparency is specified.
  HE_StateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	
  // Set the Geometry to be top level, above all other geometry in the scene.
  // This actually makes the HUD visible
  HE_StateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	
  //Does what? I'm not sure... obviously something to do with transparency.
  HE_StateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

}

HUD_Element::~HUD_Element(){
  for(unsigned int i = 0; i < textBoxes.size(); ++i){
      if(textBoxes[i])
          delete textBoxes[i];    
  }
}

osg::Geode* HUD_Element::getGeode(){
	return HE_Geode.get();
}

void HUD_Element::setElementPosition(float x, float y) {

  elementPosition.set(x, y); //Update internal position
	
  HE_Vertices->clear(); //Clear the vertices
	
  //Lower-left vertex (x, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          elementPosition.y(), 
          0
      ) 
  );

  //Lower-right vertex (x + length, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          elementPosition.y(), 
          0
      )
  );

  //Upper-right vertex (x + length, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          (elementPosition.y() + elementSize.y()), 
          0
      ) 
  );

  //Upper-left vertex (x, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          (elementPosition.y() + elementSize.y()), 
          0
      )
  );

  // Update all text box positions on screen with the index being the index
  // of this hud element
  for(unsigned int i = 0; i < textBoxes.size(); ++i){
      TextBox& t = *textBoxes[i];
      const osg::Vec2d& textPosition = t.getPosition();
      t.setPosition(
          osg::Vec3d(
          textPosition.x() + elementPosition.x(),
          textPosition.y() + elementPosition.y(),
          0
          )
      );
  }

  HE_Geode->getDrawable(0)->dirtyDisplayList();

}

void HUD_Element::setElementSize(float x, float y){

  //Update internal size
  elementSize.set(x, y);
	
  //Clear the vertices
  HE_Vertices->clear();
	
  //Lower-left vertex (x, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          elementPosition.y(), 
          0
      ) 
  );
  //Lower-right vertex (x + length, y)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          elementPosition.y(), 
          0
      )
  );
  //Upper-right vertex (x + length, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          (elementPosition.x() + elementSize.x()), 
          (elementPosition.y() + elementSize.y()), 
          0
      )
  );
  //Upper-left vertex (x, y + width)
  HE_Vertices->push_back( 
      osg::Vec3(
          elementPosition.x(), 
          (elementPosition.y() + elementSize.y()), 
          0
      )
  );
}

void HUD_Element::setElementColor(float r, float g, float b, float a){
  //Set our internal color
  elementColor.set(r, g, b, a);
	
  //Clear the array of the previous color
  HE_Color->clear();
	
  //Set the new color
  HE_Color->push_back(elementColor);
}

void HUD_Element::setElementAlpha(float a){
  //Set our internal color
  elementColor[3] = a;

  //Clear the array of the previous color
  HE_Color->clear();

  //Set the new color
  HE_Color->push_back(elementColor);
}

const osg::Vec2d&  HUD_Element::getElementPosition() const {
  return elementPosition;
}

const osg::Vec2d&  HUD_Element::getElementSize() const {
  return elementSize;
}

const osg::Vec4d&  HUD_Element::getElementColor() const {
  return elementColor;
}

void HUD_Element::addTextBox(TextBox& t) {
  //Push the text box into our vector of text boxes
  textBoxes.push_back(&t);
    
  //  Update the index position of the text box with the position of this
  //  hud element's index
  const osg::Vec2d& textPosition = t.getPosition();
  t.setPosition(
      osg::Vec3d(
          textPosition.x() + elementPosition.x(),
          textPosition.y() + elementPosition.y(),
          0
      )
  );
    
  //Attach the text object to the geode as a drawable
  HE_Geode->addDrawable(&t.getTextObject());
}

TextBox* HUD_Element::getTextBox(unsigned int id) const {
  // Return a text box with the id if one exists. Note that multiple text box
  // with the same id can exist, thus this will return the first one added with
  // a given id.
  for(unsigned int i = 0; i < textBoxes.size(); ++i){
      if(textBoxes[i]->getId() == id){
          return textBoxes[i];
      }
  }
  return NULL;
}
