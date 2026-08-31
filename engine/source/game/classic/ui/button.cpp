#include "button.h"

using namespace OSS;
using namespace Classic;


Button::Button()
{
	pressed = false;
	disabled = false;
}

void Button::draw(rectd dirtyRect)
{
	TexturedQuad quad;
	quad.rect.size = getFrameSize();
	quad.texture = (disabled ? disabledTexture : (pressed ? pressedTexture : normalTexture));
	quad.textureRect = (disabled ? disabledTextureRect : (pressed ? pressedTextureRect : normalTextureRect));
	
	//A state without art would otherwise draw as a white box, which looks
	//like a broken texture rather than a missing one.  Falling back keeps the
	//button readable and makes the omission show up as "never highlights".
	if (!quad.texture) {
		quad.texture = normalTexture;
		quad.textureRect = normalTextureRect;
	}
	
	quad.draw();
	
	View::draw(dirtyRect);
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Events
//----------------------------------------------------------------------------------------------------

bool Button::eventMouseDown(MouseButtonEvent * event)
{
	//Ignore the mouse down if it occured outside of our frame or we're disabled.
	if (disabled)
		return false;
	if (!getFrame().containsPoint(getSuperview()->convertFrom(event->position, NULL)))
		return false;
	
	//We were clicked.
	onClicked();
	
	return true;
}
