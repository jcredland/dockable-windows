/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

void DockableWindowManager::createHeavyWeightWindow(DockableComponent * comp, const Point<int> &screenPosition)
{
	auto window = new ResizableWindow(comp->getName(), true);
	window->setContentNonOwned(comp, true);
	window->setTopLeftPosition(screenPosition);
	windows.add(window);
	window->setVisible(true);
}

DockBase::DockBase(DockableWindowManager& manager_, Component * dockComponent_)
	:
	manager(manager_),
	dockComponent(dockComponent_)
{
	manager.addDock(this);
}


DockBase::~DockBase()
{
	manager.removeDock(this);
}

bool DockBase::containsScreenPosition(const Point<int>& screenPosition) const
{
	auto screenBounds = dockComponent->getScreenBounds();
	return screenBounds.contains(screenPosition);
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

DockableWindowManager::TargetOutline::TargetOutline(Image image_) :
	TopLevelWindow("Window Being Dragged", true), image(image_)
{
	setOpaque(false);
}

//int DockableWindowManager::TargetOutline::getDesktopWindowStyleFlags() const
//{
//	return TopLevelWindow::getDesktopWindowStyleFlags() || ComponentPeer::windowIsSemiTransparent;
//}

void DockableWindowManager::TargetOutline::paint(Graphics& g)
{
	g.setColour(Colours::blue.withAlpha(0.5f));
	g.drawImageAt(image, 0, 0, false);
}

DockBase* DockableWindowManager::getDockUnderScreenPosition(Point<int> position)
{
	for (auto d : docks)
		if (d->containsScreenPosition(position))
			return d;

	return nullptr;
}

void DockableWindowManager::handleComponentDragEnd(DockableComponent* component, const Point<int> & screenPosition)
{
	divorceComponentFromParent(component);

	if (highlightedDock)
		highlightedDock->hideDockableComponentPlacement();

	highlightedDock = nullptr;

	auto targetDock = getDockUnderScreenPosition(screenPosition);

	if (!targetDock)
		createHeavyWeightWindow(component, screenPosition);
	else
		targetDock->attachDockableComponent(component, screenPosition);

	currentlyDraggedComponent = nullptr;
}

void DockableWindowManager::handleComponentDrag(DockableComponent * componentBeingDragged, Point<int> location, int w, int h)
{
	if (!targetOutline)
	{
		auto image = componentBeingDragged->createComponentSnapshot(componentBeingDragged->getLocalBounds());
		targetOutline = new TargetOutline(image);
	}

	auto dock = getDockUnderScreenPosition(location);

	if (dock)
	{
		dock->showDockableComponentPlacement(componentBeingDragged, location);

		if (dock != highlightedDock && highlightedDock)
			highlightedDock->hideDockableComponentPlacement();

		highlightedDock = dock;
	}

	targetOutline->setBounds(location.getX(), location.getY(), w, h);
	targetOutline->setVisible(true);

	if (componentBeingDragged != currentlyDraggedComponent)
	{
		auto parentDock = componentBeingDragged->getCurrentDock();

		if (parentDock)
			parentDock->startDockableComponentDrag(componentBeingDragged);

		currentlyDraggedComponent = componentBeingDragged;
	}
}

void DockableWindowManager::clearTargetPosition()
{
	targetOutline = nullptr;
}

void DockableWindowManager::divorceComponentFromParent(DockableComponent* component)
{
	ResizableWindow* foundWindow{ nullptr };

	for (auto w : windows)
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
		for (auto d : docks)
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

DockBase* DockableComponent::getCurrentDock() const
{
	auto parent = getParentComponent();

	while (parent != nullptr)
	{
		auto dockBase = manager.getDockBaseForComponent(parent);

		if (dockBase)
			return dockBase;

		parent = parent->getParentComponent();
	}

	return nullptr; // it's a window and not docked!
}

DockableComponentTitleBar::DockableComponentTitleBar(DockableComponent& owner_, DockableWindowManager& manager_) :
	owner(owner_),
	manager(manager_)
{
	setMouseCursor(MouseCursor::DraggingHandCursor);
}

void DockableComponentTitleBar::paint(Graphics& g)
{
	g.fillAll(Colours::grey);
	g.setFont(Font(12.0).boldened());
	g.drawText("Title Bar", getLocalBounds(), Justification::centred, false);

	g.setColour(Colours::black.withAlpha(0.2f));
	g.drawHorizontalLine(getHeight() - 1, 0.0f, float(getWidth()));

	g.setColour(Colours::lightgrey.withAlpha(0.2f));
	g.drawHorizontalLine(0, 0.0f, float(getWidth()));
}

void DockableComponentTitleBar::mouseDrag(const MouseEvent& e)
{
	auto windowPosition = e.getScreenPosition();
	manager.handleComponentDrag(&owner, windowPosition, owner.getWidth(), owner.getHeight());
}

void DockableComponentTitleBar::mouseUp(const MouseEvent& e)
{
	manager.clearTargetPosition();
	manager.handleComponentDragEnd(&owner, e.getScreenPosition());
}

WindowDockVertical::WindowDockVertical(DockableWindowManager& manager_)
	: 
	DockBase(manager_, this),
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
	auto area = getLocalBounds();

	for (auto d : dockedComponents)
	{
		if (!d->isVisible())
			continue;

		// intelligent resize code codes here
		d->setBounds(area.withHeight(windowHeight));
		area.translate(0, windowHeight);
	}
}

void WindowDockVertical::paint(Graphics& g)
{
	g.fillAll(Colours::black);

}

void WindowDockVertical::paintOverChildren(Graphics& g)
{
	if (highlight)
	{
		g.setColour(Colours::red);
		g.fillRect(0, highlightYPosition, getWidth(), 2);
	}
}

int WindowDockVertical::getPlacementPositionForPoint(Point<int> pointRelativeToComponent) const
{
	int result{0};
	auto target = pointRelativeToComponent.getY();
	auto distance = abs(result - target);

	for (auto c: dockedComponents)
	{
		auto compBottom = c->getBounds().getBottom();
		auto newDistance = abs(compBottom - target);

		if (newDistance < distance)
		{
			result = compBottom;
			distance = newDistance;
		}
	}

	return result;
}

void WindowDockVertical::startDockableComponentDrag(DockableComponent* component)
{
	component->setVisible(false);
	resized();
}

void WindowDockVertical::detachDockableComponent(DockableComponent* component)
{
	if (!dockedComponents.contains(component))
		return;

	removeChildComponent(component);
	dockedComponents.removeAllInstancesOf(component);
}

void WindowDockVertical::hideDockableComponentPlacement()
{
	highlight = false;
	repaint();
}

void WindowDockVertical::showDockableComponentPlacement(DockableComponent*, Point<int> position)
{
	highlight = true;
	highlightYPosition = getPlacementPositionForPoint(getLocalPoint(nullptr, position));
	repaint();
}

bool WindowDockVertical::attachDockableComponent(DockableComponent* component, Point<int>)
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
	dock.addComponentToDock(new ExampleDockableComponent(dockManager, Colours::blue));
	dock.addComponentToDock(new ExampleDockableComponent(dockManager, Colours::blueviolet));
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
	auto area = getLocalBounds();
	dock.setBounds(area.withWidth(150));
}