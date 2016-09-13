/*

  ==============================================================================

	JAdvancedDock.h
	Created: 6 Jul 2016 10:27:02pm
	Author:  jim

  ==============================================================================
*/

#ifndef ADVANCEDDOCK_H_INCLUDED
#define ADVANCEDDOCK_H_INCLUDED

#include "JDockableWindows.h"

namespace AdvancedDockPlaces
{
	enum Places
	{
		top, left, right, bottom, centre,
		none
	};
};

class AdvancedDockPlacementDialog;

/**
The advanced dock allows vertical and horizontal splits, as well as tabs.
*/
class JAdvancedDock
	:
	public Component,
	DockBase
{
public:
	JAdvancedDock(DockableWindowManager& manager_);

	~JAdvancedDock();;

	void resized() override;
	void paint(Graphics& g) override;

private:
	struct WindowLocation
	{
		WindowLocation(int y, int x, int t);
		int y{ 0 };
		int x{ 0 };
		int tab{ 0 };
	};

	WindowLocation getWindowLocationAtPoint(const Point<int>& screenPosition);
	Rectangle<int> getWindowBoundsAtPoint(const Point<int>& p);
	/**
	Insert a new window in to the right place in our dock...
	*/
	void insertWindow(const Point<int>& screenPos, AdvancedDockPlaces::Places places, DockableComponentWrapper* comp);

	void showDockableComponentPlacement(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void hideDockableComponentPlacement() override;
	void startDockableComponentDrag(DockableComponentWrapper* component) override;
	void insertNewDock(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc);
	void insertNewRow(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc);
	void insertToNewTab(DockableComponentWrapper* comp, JAdvancedDock::WindowLocation loc);
	bool attachDockableComponent(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void detachDockableComponent(DockableComponentWrapper* component) override;
	void revealComponent(DockableComponentWrapper* dockableComponent) override;


	class RowType;

	std::vector<RowType> rows;
	std::vector<std::unique_ptr<StretchableLayoutResizerBar>> resizers;
	StretchableLayoutManager layout;

	void rebuildRowResizers();
	void layoutRows(const Rectangle<int>& area);


	ScopedPointer<AdvancedDockPlacementDialog> placementDialog;
};


#endif  // ADVANCEDDOCK_H_INCLUDED
