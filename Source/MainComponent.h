
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class DockableWindowManager;
class DockableComponent;

class DockBase
{
public:
	DockBase(DockableWindowManager & manager_);

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
	virtual void stopShowingComponentPlacement() = 0;
	/** 
	Called when the user drags a window over the dock and releases the mouse, use this to place the 
	window into the dock.  Return false if the DockableComponent cannot be added, and a new window will be
	created instead.
	*/
	virtual bool addDockableComponent(DockableComponent* component, Point<int> screenPosition) = 0;
	/** 
	Should remove the component from the dock if it's present, and resize or rearrange any other windows 
	accordingly.
	*/
	virtual void detachDockableComponent(DockableComponent* component) = 0;

private:
	DockableWindowManager & manager;
};

class DockableWindowManager
{
public:
	DockableWindowManager();

	class TargetOutline 
		: 
		public TopLevelWindow
	{
	public:
		TargetOutline();
		void paint(Graphics& g) override;
	};

	/** 
	Shows an outline when a component is being dragged. 
	*/
	void showTargetPosition(Point<int> location, int w, int h);

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
	void placeComponent(DockableComponent* component, Point<int> position);


private:
	friend class DockBase;
	void addDock(DockBase* newDock);
	void removeDock(DockBase* dockToRemove);

	void createHeavyWeightWindow(DockableComponent * comp);

	OwnedArray<ResizableWindow> windows;
	ScopedPointer<TargetOutline> targetOutline;
	Array<DockBase *> docks;
	DockBase * highlightedDock{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DockableWindowManager)
};


class DockableComponentTitleBar;

/** Base class for windows that can be dragged between docks and desktop windows.  */
class DockableComponent
	:
	public Component
{
public:
	DockableComponent(DockableWindowManager &);

	void resized() override;

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


class ExampleDockableWindow
	:
	public DockableComponent
{
public:
	ExampleDockableWindow(DockableWindowManager& dockableWindowManager)
		: DockableComponent(dockableWindowManager)
	{}

	void paint(Graphics & g) override
	{
		g.fillAll(Colours::blue);
		g.drawText("Window Content", getLocalBounds(), Justification::centred, false);
	}
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

private:
	void showDockableComponentPlacement(DockableComponent* component, Point<int> screenPosition) override;
	bool addDockableComponent(DockableComponent* component, Point<int> screenPosition) override;
	void detachDockableComponent(DockableComponent* component) override;
	void stopShowingComponentPlacement() override;

	bool highlight{ false };
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