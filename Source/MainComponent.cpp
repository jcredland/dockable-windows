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

DockableComponent* DockableWindowManager::createDockableComponent(Component* component)
{
	auto d = new DockableComponent(*this, component);
	dockableComponents.add(d);
	return d;
}

DockBase* DockableWindowManager::getDockWithComponent(Component* component) const
{
	for (auto d: docks)
		if (d->isUsingComponent(component))
			return d;

	return nullptr;
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
	setInterceptsMouseClicks(false, true);
	titleBar = new DockableComponentTitleBar(*this, manager);
	addAndMakeVisible(titleBar);
}

DockableComponent::DockableComponent(DockableWindowManager&manager_, Component* contentComponentUnowned)
	:
	manager(manager_)
{
	setInterceptsMouseClicks(false, true);
	titleBar = new DockableComponentTitleBar(*this, manager);
	addAndMakeVisible(titleBar);

	setContentComponentUnowned(contentComponentUnowned);
}

void DockableComponent::resized()
{
	auto area = getLocalBounds();
	titleBar->setBounds(area.removeFromTop(20));
	
	if (tabButton)
	{
		auto tabArea = area.removeFromBottom(16);
		tabArea.setX(tabXPosition); 
		tabArea.setWidth(60);
		tabButton->setBounds(tabArea);
	}

	if (contentComponent)
		contentComponent->setBounds(area);
}

void DockableComponent::setContentComponentUnowned(Component* content)
{
	contentComponent = content;

	if (contentComponent)
		addAndMakeVisible(contentComponent);

	resized();
}

DockBase* DockableComponent::getCurrentDock() const
{
	auto parent = getParentComponent();

	while (parent != nullptr)
	{
		auto dockBase = manager.getDockWithComponent(parent);

		if (dockBase)
			return dockBase;

		parent = parent->getParentComponent();
	}

	return nullptr; // it's a window and not docked!
}

void DockableComponent::setShowTab(bool shouldShowTab, int xPos_, bool isFrontTab_)
{
	if (shouldShowTab)
	{
		tabButton = new DockableComponentTab(*this, manager);
		addAndMakeVisible(tabButton);
		tabXPosition = xPos_;
		tabButton->setIsFrontTab(isFrontTab_);
	}
	else
	{
		tabButton = nullptr;
	}

	resized();
}

Rectangle<int> DockableComponent::getTabButtonBounds() const
{
	if (tabButton)
		return tabButton->getBounds();

	return {};
}

DockableComponentDraggable::DockableComponentDraggable(DockableComponent& owner_, DockableWindowManager& manager_) :
	owner(owner_),
	manager(manager_)
{
	setMouseCursor(MouseCursor::DraggingHandCursor);
}

DockableComponentTitleBar::DockableComponentTitleBar(DockableComponent& owner_, DockableWindowManager& manager_):
	DockableComponentDraggable(owner_, manager_)
{
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

DockableComponentTab::DockableComponentTab(DockableComponent& owner_, DockableWindowManager& manager_):
	DockableComponentDraggable(owner_, manager_)
{
}

void DockableComponentTab::paint(Graphics& g)
{
	g.fillAll(frontTab ? Colours::grey : Colours::darkgrey);

	g.setFont(Font(12.0).boldened());
	g.drawText("Tab", getLocalBounds(), Justification::centred, false);
}

void DockableComponentTab::setIsFrontTab(bool nowFrontTab)
{
	frontTab = nowFrontTab;
	repaint();
}

void DockableComponentTab::mouseUp(const MouseEvent& e)
{
	if (!isDragging())
		getDockableComponent().tabButtonClicked();
	else
		DockableComponentDraggable::mouseUp(e);
}

void DockableComponentDraggable::mouseDrag(const MouseEvent& e)
{
	if (dragging || e.getDistanceFromDragStart() > 10)
	{
		auto windowPosition = e.getScreenPosition();
		manager.handleComponentDrag(&owner, windowPosition, owner.getWidth(), owner.getHeight());
		dragging = true;
	}
}

void DockableComponentDraggable::mouseUp(const MouseEvent& e)
{
	manager.clearTargetPosition();
	manager.handleComponentDragEnd(&owner, e.getScreenPosition());
	dragging = false;
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

void WindowDockVertical::addComponentToDock(Component* comp)
{
	auto dockable = manager.createDockableComponent(comp);
	dockedComponents.add(dockable);
	addAndMakeVisible(dockable);
	resized();
}

void WindowDockVertical::resized()
{
	auto windowHeight = 110;
	auto area = getLocalBounds();

	for (auto d : dockedComponents)
	{
		d->setShowTab(false, 0, false);

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

WindowDockVertical::PlacementPosition WindowDockVertical::getPlacementPositionForPoint(Point<int> pointRelativeToComponent) const
{
	int result{0};
	int componentIndex{0};

	auto target = pointRelativeToComponent.getY();
	auto distance = abs(result - target);

	int count{ 0 };

	for (auto c: dockedComponents)
	{
		count++;

		auto compBottom = c->getBounds().getBottom();
		auto newDistance = abs(compBottom - target);

		if (newDistance < distance)
		{
			result = compBottom;
			distance = newDistance;
			componentIndex = count;
		}
	}

	return{ result, componentIndex };
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
	auto pos = getPlacementPositionForPoint(getLocalPoint(nullptr, position));
	highlightYPosition = pos.yPosition;
	repaint();
}

bool WindowDockVertical::attachDockableComponent(DockableComponent* component, Point<int> position)
{
	addAndMakeVisible(component);
	auto pos = getPlacementPositionForPoint(getLocalPoint(nullptr, position));
	dockedComponents.insert(pos.insertAfterComponentIndex, component);
	resized();
	return true;
}

//==============================================================================

MainContentComponent::MainContentComponent()
{
	setSize(600, 400);
	addAndMakeVisible(dock);
	addAndMakeVisible(tabDock);
	auto baseColour = Colours::blue.withSaturation(0.4f).withBrightness(0.4f);
	dock.addComponentToDock(new ExampleDockableComponent(baseColour.withRotatedHue(0.1f)));
	dock.addComponentToDock(new ExampleDockableComponent(baseColour.withRotatedHue(0.2f)));
	dock.addComponentToDock(new ExampleDockableComponent(baseColour.withRotatedHue(0.3f)));

	tabDock.addComponentToDock(new ExampleDockableComponent(baseColour.withRotatedHue(0.6f)));
	tabDock.addComponentToDock(new ExampleDockableComponent(baseColour.withRotatedHue(0.7f)));
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
	dock.setBounds(area.removeFromLeft(150));
	tabDock.setBounds(area.removeFromRight(250));
}