/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

void DockableWindowManager::createHeavyWeightWindow(DockableComponent * comp)
{
	auto window = new ResizableWindow(comp->getName(), true);
	window->setContentNonOwned(comp, true);
	windows.add(window);
	window->setVisible(true);
}

DockBase::DockBase(DockableWindowManager& manager_)
	:
	manager(manager_)
{
	manager.addDock(this);
}

DockBase::~DockBase()
{
	manager.removeDock(this);
}

DockableWindowManager::DockableWindowManager()
{
}

void DockableWindowManager::addDock(DockBase* newDock)
{
	docks.addIfNotAlreadyThere(newDock);
}

void DockableWindowManager::removeDock(DockBase* dockToRemove)
{
	docks.removeAllInstancesOf(dockToRemove);
}

DockableWindowManager::TargetOutline::TargetOutline():
	TopLevelWindow("Temp", true)
{
}

void DockableWindowManager::TargetOutline::paint(Graphics& g)
{
	g.fillAll(Colours::red);
	g.setColour(Colours::white);
	g.drawText("Window Being Draggged", getLocalBounds(), Justification::centred, false);
}

DockBase* DockableWindowManager::getDockUnderScreenPosition(Point<int> position)
{
	for (auto d: docks)
	{
		auto c = dynamic_cast<Component *>(d);

		if (c)
		{
			auto screenBounds = c->getScreenBounds();

			if (screenBounds.contains(position))
				return d;
		}
		else
		{
			jassertfalse; // docks should be components..
		}
	}

	return nullptr;
}

void DockableWindowManager::placeComponent(DockableComponent* component, Point<int> screenPosition)
{
	divorceComponentFromParent(component);

	auto targetDock = getDockUnderScreenPosition(screenPosition);

	if (!targetDock)
		createHeavyWeightWindow(component);
	else
		targetDock->addDockableComponent(component, screenPosition);
}

void DockableWindowManager::showTargetPosition(Point<int> location, int w, int h)
{
	if (!targetOutline)
		targetOutline = new TargetOutline();

	targetOutline->setBounds(location.getX(), location.getY(), w, h);
	targetOutline->setVisible(true);
}

void DockableWindowManager::clearTargetPosition()
{
	targetOutline = nullptr;
}

void DockableWindowManager::divorceComponentFromParent(DockableComponent* component)
{
	ResizableWindow* foundWindow{nullptr};

	for (auto w: windows)
	{
		if (w->getContentComponent() == component)
			foundWindow = w;
	}

	if (foundWindow)
	{
		windows.removeObject(foundWindow);
	}
	else
	{
		for (auto d: docks)
			d->detachDockableComponent(component);
	}
}

//==============================================================================

DockableComponent::DockableComponent(DockableWindowManager & manager_)
	:
	manager(manager_)
{
	titleBar = new DockableComponentTitleBar(*this, manager);
	addAndMakeVisible(titleBar);
}

void DockableComponent::resized()
{
	titleBar->setBounds(getLocalBounds().withHeight(20));
}

DockableComponentTitleBar::DockableComponentTitleBar(DockableComponent& owner_, DockableWindowManager& manager_):
	owner(owner_),
	manager(manager_)
{
	setMouseCursor(MouseCursor::DraggingHandCursor);
}

void DockableComponentTitleBar::paint(Graphics& g)
{
	g.fillAll(Colours::darkgrey);
}

void DockableComponentTitleBar::mouseDrag(const MouseEvent& e)
{
	auto windowPosition = e.getScreenPosition();
	manager.showTargetPosition(windowPosition, owner.getWidth(), owner.getHeight());
}

void DockableComponentTitleBar::mouseUp(const MouseEvent& e)
{
	manager.clearTargetPosition();
	manager.placeComponent(&owner, e.getScreenPosition());
}

WindowDockVertical::WindowDockVertical(DockableWindowManager& manager_)
	: DockBase(manager_),
	manager(manager_)
{
}

WindowDockVertical::~WindowDockVertical()
{
}

void WindowDockVertical::addComponentToDock(DockableComponent* comp)
{
	dockedComponents.add(comp);
	addAndMakeVisible(comp);
	resized();
}

void WindowDockVertical::resized()
{
	auto windowHeight = 110;
	auto bounds = getLocalBounds();

	for (auto d : dockedComponents)
	{
		// intelligent resize code codes here
		d->setBounds(bounds.withHeight(windowHeight));
		bounds.translate(0, windowHeight);
	}
}

void WindowDockVertical::paint(Graphics& g)
{
	g.fillAll(Colours::black);

	if (highlight)
	{
		g.setColour(Colours::red);
		g.drawRect(getLocalBounds(), 2.0f);
	}
}

void WindowDockVertical::detachDockableComponent(DockableComponent* component)
{
	if (!dockedComponents.contains(component))
		return;

	removeChildComponent(component);
	dockedComponents.removeAllInstancesOf(component);
}

void WindowDockVertical::stopShowingComponentPlacement()
{
	highlight = false;
	repaint();
}

void WindowDockVertical::showDockableComponentPlacement(DockableComponent* component, Point<int> screenPosition)
{
	highlight = true;
}

bool WindowDockVertical::addDockableComponent(DockableComponent* component, Point<int> screenPosition)
{
	addAndMakeVisible(component);
	dockedComponents.add(component);
	resized();
	return true;
}

//==============================================================================

MainContentComponent::MainContentComponent()
{
	setSize(600, 400);
	addAndMakeVisible(dock);
	dock.addComponentToDock(new ExampleDockableWindow(dockManager));
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint(Graphics& g)
{
	g.fillAll(Colour(0xff001F36));
}

void MainContentComponent::resized()
{
	auto bounds = getLocalBounds();
	dock.setBounds(bounds.withWidth(150));
}