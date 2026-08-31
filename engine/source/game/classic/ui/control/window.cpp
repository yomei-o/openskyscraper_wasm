#include "window.h"

#include "../game.h"

using namespace OSS;
using namespace Classic;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Construction
//----------------------------------------------------------------------------------------------------

ControlWindow::ControlWindow(GameUI * ui) : ui(ui)
{
	//Since the control window is fixed in size, we may set the frame size right from the beginning.
	setFrameSize(double2(431, 41));
	
	//Fetch the background texture of the control window
	backgroundTexture = Texture::named("simtower/ui/control/background");
	
	//Calculate the watch's rect
	rectd watchRect(double2(), getFrameSize());
	watchRect.inset(double2(6, 6));
	watchRect.size.x = watchRect.size.y;
	
	//Initialize the watch view
	watchView = new WatchView(this);
	watchView->setFrame(watchRect);
	addSubview(watchView);
	
	//Initialize the rating view
	ratingView = new RatingView(this);
	ratingView->setFrameOrigin(double2(45, 17));
	addSubview(ratingView);
}

Tower * ControlWindow::getTower()
{
	return ui->getTower();
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Drawing
//----------------------------------------------------------------------------------------------------

void ControlWindow::draw(rectd dirtyRect)
{
	//Create a textured quad and make it fill our entire frame
	TexturedQuad quad;
	quad.texture = backgroundTexture;
	quad.rect.size = getFrameSize();
	
	//Draw the background quad
	quad.draw();
	
	//Fill in the Fund and Pop fields.  SimTower drew these with a GDI font
	//and the background bitmap carries only the two labels, so the values
	//have to be drawn here.  The rects come from measuring the labels in the
	//background: their baselines sit at y 25 and 8 in the window, and the
	//fields run out to x 425.
	Tower * tower = getTower();
	if (tower) {
		color4d ink = (color4d){0.1, 0.1, 0.1, 1};
		BitmapFont::drawRightAligned(
			BitmapFont::formatMoney(tower->funds->getFunds()),
			double2(425, 25), ink);
		BitmapFont::drawRightAligned(
			BitmapFont::formatNumber(tower->environment->getPopulation()),
			double2(425, 8), ink);
	}
	
	//Call the original implementation of this method which will also draw the subviews
	Window::draw(dirtyRect);
}
