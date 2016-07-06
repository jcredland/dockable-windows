
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class DockableWindowManager;
class DockableComponent;

/**
Base for things that can act as docks for DockableComponents 
*/
class DockBase
{
public:
	DockBase(DockableWindowManager & manager_, Component * dockComponent);

	virtual ~DockBase();
	/** 
	Called when the user is dragging over the dock.  This should do something to highlight where the 
	component may be placed, or do nothing if the component cannot be placed here.
	*/
	virtual void showDockableComponentPlacement(DockableComponent* component, Point<int> screenPosition) = 0;
	/**
	Called when the dragging has stopped or moved away from the component and we don't need to show
	the highlighted position any more.  
	*/
	virtual void hideDockableComponentPlacement() = 0;
	/**
	Mark or otherwise show that the component is being moved from the dock.    E.g. by making the component
	grey or invisible.   If the user decides not to drag the component after all attachDockableComponent 
	will be called.  
	*/
	virtual void startDockableComponentDrag(DockableComponent* component) = 0;
	/** 
	Called when the user drags a window over the dock and releases the mouse, use this to place the 
	window into the dock.  Return false if the DockableComponent cannot be added, and a new window will be
	created instead.
	*/
	virtual bool attachDockableComponent(DockableComponent* component, Point<int> screenPosition) = 0;
	/** 
	Should remove the component from the dock if it's present, and resize or rearrange any other windows 
	accordingly.
	*/
	virtual void detachDockableComponent(DockableComponent* component) = 0;

	/** 
	Tests to see if this Dock contains the provided screen position.  Used internally 
	while drags are in progress. 
	*/
	bool containsScreenPosition(const Point<int>& screenPosition) const;

	bool isUsingComponent(Component * c) const { return dockComponent == c; }
private:
	DockableWindowManager & manager;
	Component* dockComponent;
};

/**
The DockableWindowManager controls the attaching and removing of DockableWindow objects from DockBase 
implementations and top level windows.
*/
class DockableWindowManager
{
public:
	DockableWindowManager();

	class TargetOutline 
		: 
		public TopLevelWindow
	{
	public:
		TargetOutline(Image imageForDraggingWindow);

		//int getDesktopWindowStyleFlags() const override;
		void paint(Graphics& g) override;
		Image image;
	};

	/** 
	Shows an outline of the component while it's being dragged, and highlight any 
	docks the component is dragged over.
	*/
	void handleComponentDrag(DockableComponent *, Point<int> location, int w, int h);


	/** 
	Removes the outline when the mouse is released. 
	*/
	void clearTargetPosition();
	
	/** 
	Removes the component from it's parent, prior to rehoming it. 
	*/
	void divorceComponentFromParent(DockableComponent* component);

	/** 
	Returns the dock under the provided screen position, if there is no 
	dock returns nullptr.

	@note may return the wrong result if there are overlapping docks.
	*/
	DockBase* getDockUnderScreenPosition(Point<int> position);

	/**
	Called to put a component in new screen position.  Assesses whether this is 
	a dock, a tab or a top level window and attaches the window to the new 
	component.
	*/
	void handleComponentDragEnd(DockableComponent* component, const Point<int> & screenPosition);


	DockBase * getDockBaseForComponent(Component * component) const
	{
		for (auto d: docks)
			if (d->isUsingComponent(component))
				return d;

		return nullptr;
	}
private:
	friend class DockBase;
	void addDock(DockBase* newDock);
	void removeDock(DockBase* dockToRemove);

	void createHeavyWeightWindow(DockableComponent * comp, const Point<int> & screenPosition);

	OwnedArray<ResizableWindow> windows;
	ScopedPointer<TargetOutline> targetOutline;
	Array<DockBase *> docks;
	DockBase * highlightedDock{ nullptr };
	
	Component * currentlyDraggedComponent{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DockableWindowManager)
};


class DockableComponentTitleBar;

/** 
Base class for windows that can be dragged between docks and desktop windows.  
*/
class DockableComponent
	:
	public Component
{
public:
	DockableComponent(DockableWindowManager &);

	void resized() override;

	/** 
	If the component is in a dock this returns the dock it's in.  If it's not in a dock, 
	or it's a stand alone window this returns nullptr. 
	*/
	DockBase* getCurrentDock() const;

	ScopedPointer<DockableComponentTitleBar> titleBar;
	DockableWindowManager & manager;
};


/** Title bar that implements the drag and drop functions.  */
class DockableComponentTitleBar
	:
	public Component
{
public:
	DockableComponentTitleBar(DockableComponent& owner_, DockableWindowManager& manager_);
	void paint(Graphics& g) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;
private:
	DockableComponent & owner;
	DockableWindowManager & manager;
};


/**
A really simple component we can use to test the dragging and docking.
*/
class ExampleDockableComponent
	:
	public DockableComponent
{
public:
	ExampleDockableComponent(DockableWindowManager& dockableWindowManager, const Colour &
		colour_)
		: DockableComponent(dockableWindowManager), colour(colour_)
	{}

	void paint(Graphics & g) override
	{
		g.fillAll(colour);
		g.setColour(Colours::white);
		g.drawText("Window Content", getLocalBounds(), Justification::centred, false);
	}

	Colour colour;
};


/**
A simple example dock. 
*/
class WindowDockVertical
	:
	public Component, 
	DockBase
{
public:
	WindowDockVertical(DockableWindowManager& manager_);

	~WindowDockVertical();

	/** 
		We assume you are managing the components lifetime.  However an optional change could be to have the 
		DockManager manage them. 
		*/
	void addComponentToDock(DockableComponent* comp);

	void resized() override;
	void paint(Graphics& g) override;
	void paintOverChildren(Graphics& g) override;

private:
	/** Finds the nearest top or bottom edge of an existing component to the mouse Y position */
	int getPlacementPositionForPoint(Point<int> pointRelativeToComponent) const;

	void startDockableComponentDrag(DockableComponent* component) override;
	void showDockableComponentPlacement(DockableComponent* component, Point<int> screenPosition) override;
	bool attachDockableComponent(DockableComponent* component, Point<int> screenPosition) override;
	void detachDockableComponent(DockableComponent* component) override;
	void hideDockableComponentPlacement() override;

	bool highlight{ false };
	int highlightYPosition{ 0 };

	Array<DockableComponent *> dockedComponents;
	DockableWindowManager & manager;
};


class MainContentComponent : public Component
{
public:
	//==============================================================================
	MainContentComponent();
	~MainContentComponent();

	void paint(Graphics&) override;
	void resized() override;

private:
	DockableWindowManager dockManager;
	WindowDockVertical dock { dockManager };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED