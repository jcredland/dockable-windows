
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "JDockableWindows.h"
#include "JAdvancedDock.h"


/**
A really simple component we can use to test the dragging and docking.
*/
class ExampleDockableComponent
	:
	public Component
{
public:
	ExampleDockableComponent(const String & componentName, const Colour & colour_)
		:
		colour(colour_)
	{
		Component::setName(componentName);
	}

	~ExampleDockableComponent()
	{
		jassertfalse;
	}

	void paint(Graphics & g) override
	{
		g.fillAll(colour);
		g.setColour(Colours::white);
		g.drawText("Window Content", getLocalBounds(), Justification::centred, false);
	}

private:
	Colour colour;
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
	TabDock tabDock { dockManager };
	JAdvancedDock advancedDock{ dockManager };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED