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
	void insertWindow(const Point<int>& screenPos, AdvancedDockPlaces::Places places, DockableComponentWrapper* comp);

	void showDockableComponentPlacement(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void hideDockableComponentPlacement() override;
	void startDockableComponentDrag(DockableComponentWrapper* component) override;
	bool attachDockableComponent(DockableComponentWrapper* component, Point<int> screenPosition) override;
	void detachDockableComponent(DockableComponentWrapper* component) override;
	void revealComponent(DockableComponentWrapper* dockableComponent) override
	{
		dockableComponent->toFront(true);
		resized();
	}

	typedef std::vector<std::unique_ptr<DockableComponentWrapper>> TabDockType;
	typedef std::vector<TabDockType> RowType;
	std::vector<RowType> windows;

	/** Sets up the correct tab configuration for a docked component that needs to display tabs */
	void configureTabs(const TabDockType & vector) const;

	ScopedPointer<AdvancedDockPlacementDialog> placementDialog;
};


#endif  // ADVANCEDDOCK_H_INCLUDED
