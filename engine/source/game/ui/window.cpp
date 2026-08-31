#include "window.h"

using namespace OSS;


void Window::draw(rectd dirtyRect)
{
	rectd rect;
	rect.size = getFrameSize();
	rect.inset(double2(-0.5, -0.5));
	
	glColor4f(0, 0, 0, 0.75);
	Texture::unbind();
	glBegin(GL_LINE_STRIP);
	glVertex2d(rect.minX(), rect.minY());
	glVertex2d(rect.minX(), rect.maxY());
	glVertex2d(rect.maxX(), rect.maxY());
	glVertex2d(rect.maxX(), rect.minY());
	glVertex2d(rect.minX(), rect.minY());
	glEnd();
	
	View::draw(dirtyRect);
}


Window::Window() : dragging(false), dragOffset(0, 0) {}


bool Window::beginDrag(double2 position)
{
	//The frame lives in the superview's coordinate system, so the grab point
	//has to be converted into it before either can be compared.
	View * superview = getSuperview();
	if (!superview) return false;
	double2 p = superview->convertFrom(position, NULL);
	if (!getFrame().containsPoint(p)) return false;
	
	dragOffset = p - getFrameOrigin();
	dragging = true;
	return true;
}

bool Window::continueDrag(double2 position)
{
	if (!dragging) return false;
	View * superview = getSuperview();
	if (!superview) return false;
	
	//Keeping the grab point under the pointer is what makes a drag feel
	//attached rather than approximate.
	double2 origin = superview->convertFrom(position, NULL) - dragOffset;
	
	//Clamped to the superview: a window dragged past the edge would be
	//unreachable afterwards, since there would be nothing left to grab.
	double2 room = superview->getFrameSize() - getFrameSize();
	origin.x = std::min(std::max(origin.x, 0.0), std::max(room.x, 0.0));
	origin.y = std::min(std::max(origin.y, 0.0), std::max(room.y, 0.0));
	
	setFrameOrigin(origin);
	return true;
}

bool Window::endDrag()
{
	if (!dragging) return false;
	dragging = false;
	return true;
}