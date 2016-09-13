/*
  ==============================================================================

    JAdvancedDock.cpp
    Created: 6 Jul 2016 10:27:02pm
    Author:  jim

  ==============================================================================
*/

#include "JAdvancedDock.h"

class AdvancedDockPlacementDialog
	:
	public Component
{
public:
	AdvancedDockPlacementDialog()
	{
		for (int i = AdvancedDockPlaces::top; i <= AdvancedDockPlaces::centre; ++i)
			buttons.add(new PlacementButton(AdvancedDockPlaces::Places(i)));

		for (auto b : buttons)
			addAndMakeVisible(b);
	}


	void setLeftRightVisible(bool nowVisible) 
	{
		buttons[AdvancedDockPlaces::left]->setVisible(nowVisible);
		buttons[AdvancedDockPlaces::right]->setVisible(nowVisible);
	}

	void setTopBottomVisible(bool nowVisible) 
	{
		buttons[AdvancedDockPlaces::top]->setVisible(nowVisible);
		buttons[AdvancedDockPlaces::bottom]->setVisible(nowVisible);
	}

	void setMousePosition(const Point<int>& screenPos)
	{
		auto p = getLocalPoint(nullptr, screenPos);

		for (auto b : buttons)
			b->setIsMouseOver(b->contains(b->getLocalPoint(this, p)));
		
		repaint();
	}

	class PlacementButton : public Component
	{
	public:
		PlacementButton(AdvancedDockPlaces::Places place): place(place) {}

		void paint(Graphics & g) override
		{
			g.setColour(Colours::white);

			if (mouseOver)
				g.fillRect(getLocalBounds().toFloat().reduced(2.0f));
			else
				g.drawRect(getLocalBounds().toFloat().reduced(2.0f), 1.0f);

			g.fillRect(getLocalBounds().toFloat().reduced(2.0f).withHeight(3.0f));
		}


		void setIsMouseOver(bool isOver)
		{
			if (isOver != mouseOver)
			{
				mouseOver = isOver;
				repaint();
			}
		};

		bool mouseOver{false};
		AdvancedDockPlaces::Places place;
	};

	void resized() override
	{
		auto area = getLocalBounds().toFloat().reduced(padding);
		auto h3 = area.getHeight() / 3.0f;
		auto w3 = area.getWidth() / 3.0f;

		topArea = area.removeFromTop(h3).translated(h3, 0.0f).withWidth(h3);
		buttons[AdvancedDockPlaces::top]->setBounds(topArea.toNearestInt());

		midArea = area.removeFromTop(h3);

		auto midAreaChop = midArea;

		buttons[AdvancedDockPlaces::left]->setBounds(midAreaChop.removeFromLeft(w3).toNearestInt());
		centreArea = midAreaChop.removeFromLeft(w3);
		buttons[AdvancedDockPlaces::centre]->setBounds(centreArea.toNearestInt());
		buttons[AdvancedDockPlaces::right]->setBounds(midAreaChop.toNearestInt());

		bottomArea = area.translated(h3, 0.0f).withWidth(h3);
		buttons[AdvancedDockPlaces::bottom]->setBounds(bottomArea.toNearestInt());
	}



	AdvancedDockPlaces::Places getSelectionForCoordinates(Point<int> position) const
	{
		for (auto b : buttons)
			if (b->getBounds().contains(position) && b->isVisible())
				return b->place;

		return AdvancedDockPlaces::none;
	}

	void paint(Graphics & g) override
	{
		g.setColour(Colours::black);
		g.beginTransparencyLayer(0.4f);

		if (buttons[AdvancedDockPlaces::top]->isVisible() && buttons[AdvancedDockPlaces::left]->isVisible())
		{
			g.fillRoundedRectangle(topArea.expanded(padding), rounding);
			g.fillRoundedRectangle(midArea.expanded(padding), rounding);
			g.fillRoundedRectangle(bottomArea.expanded(padding), rounding);
		}
		else
		{
			g.fillRoundedRectangle(centreArea.expanded(padding), rounding);
		}

		g.endTransparencyLayer();
	}

private:
	Rectangle<float> topArea, midArea, bottomArea, centreArea;
	OwnedArray<PlacementButton> buttons;
	const float rounding = 4.0f;
	const float padding = 4.0f;
};

JAdvancedDock::JAdvancedDock(DockableWindowManager& manager_): DockBase(manager_, this)
{
	placementDialog = new AdvancedDockPlacementDialog();
	addChildComponent(placementDialog);
}

JAdvancedDock::~JAdvancedDock()
{
}

JAdvancedDock::WindowLocation::WindowLocation(int y, int x, int t): y(y), x(x), tab(t)
{
}

JAdvancedDock::WindowLocation JAdvancedDock::getWindowLocationAtPoint(const Point<int>& screenPosition)
{
	auto localPos = getLocalPoint(nullptr, screenPosition);

	for (int rowNumber = 0; rowNumber < windows.size(); ++rowNumber)
	{
		auto& row = windows[rowNumber];

		for (int colNumber = 0; colNumber < row.size(); ++colNumber)
		{
			auto& col = row[colNumber];

			for (int tabNumber = 0; tabNumber < col.size(); ++tabNumber)
			{
				auto& tab = col[tabNumber];

				if (tab->getBounds().contains(localPos))
					return {rowNumber, colNumber, tabNumber};
			}
		}
	}

	return {0,0,0};
}

Rectangle<int> JAdvancedDock::getWindowBoundsAtPoint(const Point<int>& p)
{
	auto loc = getWindowLocationAtPoint(p);

	DBG(String(loc.x) + " " + String(loc.y));

	if (windows.size() == 0)
		return getLocalBounds();

	return windows[loc.y][loc.x][loc.tab]->getBounds();
}

void JAdvancedDock::showDockableComponentPlacement(DockableComponentWrapper* component, Point<int> screenPosition)
{
	placementDialog->setTopBottomVisible(windows.size() > 0);
	placementDialog->setLeftRightVisible(windows.size() > 0);

	placementDialog->setVisible(true);
	placementDialog->toFront(false);
	placementDialog->setBounds(getWindowBoundsAtPoint(screenPosition).withSizeKeepingCentre(100, 100));
	placementDialog->setMousePosition(screenPosition);
}

void JAdvancedDock::hideDockableComponentPlacement()
{
	placementDialog->setVisible(false);
}

void JAdvancedDock::startDockableComponentDrag(DockableComponentWrapper* component)
{
}

void JAdvancedDock::insertWindow(const Point<int>& screenPos, AdvancedDockPlaces::Places places, DockableComponentWrapper* comp)
{
	auto loc = getWindowLocationAtPoint(screenPos);

	switch (places)
	{

		// insert a new row...

	case AdvancedDockPlaces::top:
		{
			RowType newRow;
			newRow.push_back(TabDockType());
			newRow[0].push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			windows.insert(windows.begin() + loc.y, std::move(newRow));
			break;
		}

	case AdvancedDockPlaces::bottom:
		{
			auto nudge = windows.size() > 0 ? 1 : 0;
			RowType newRow;
			newRow.push_back(TabDockType());
			newRow[0].push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			windows.insert(windows.begin() + loc.y + nudge, std::move(newRow));
			break;
		}

		// add a new dock to an existing row...

	case AdvancedDockPlaces::left:
		{
			auto& row = windows[loc.y];
			TabDockType newTabDock;
			newTabDock.push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			row.insert(row.begin() + loc.x, std::move(newTabDock));
		}
		break;

	case AdvancedDockPlaces::right:
		{
			auto& row = windows[loc.y];
			TabDockType newTabDock;
			newTabDock.push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			row.insert(row.begin() + loc.x + 1, std::move(newTabDock));
		}
		break;

		// make or add to a tabbed dock...

	case AdvancedDockPlaces::centre:
		if (windows.size() > 0)
		{
			auto & location = windows[loc.y][loc.x];
			location.push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			configureTabs(location);
		}
		else
		{
			RowType newRow;
			newRow.push_back(TabDockType());
			newRow[0].push_back(std::unique_ptr<DockableComponentWrapper>(comp));
			windows.push_back(std::move(newRow));
			break;
		}
		break;

	case AdvancedDockPlaces::none:
		jassertfalse;
		break;
	}
}

bool JAdvancedDock::attachDockableComponent(DockableComponentWrapper* component, Point<int> screenPosition)
{
	auto placement = placementDialog->getSelectionForCoordinates(placementDialog->getLocalPoint(nullptr, screenPosition));

	if (placement != AdvancedDockPlaces::none)
	{
		addAndMakeVisible(component);
		insertWindow(screenPosition, placement, component);
		resized();
		return true;
	}


	return false;
}

void JAdvancedDock::detachDockableComponent(DockableComponentWrapper* component)
{
	for (int rowNumber = 0; rowNumber < windows.size(); ++rowNumber)
	{
		auto& row = windows[rowNumber];

		for (int colNumber = 0; colNumber < row.size(); ++colNumber)
		{
			auto& col = row[colNumber];

			for (int tabNumber = 0; tabNumber < col.size(); ++tabNumber)
			{
				auto& tab = col[tabNumber];

				if (tab.get() == component)
				{
					tab.release();

					col.erase(col.begin() + tabNumber);

					if (col.size() == 1)
					{
						/* remove tab buttons if we don't need them any more */
						col[0]->setShowTabButton(false, 0, false);
					}
					else if (col.size() == 0)
					{ 
						/* remove tabs, rows and columns if now empty... */
						row.erase(row.begin() + colNumber);

						if (row.size() == 0)
							windows.erase(windows.begin() + rowNumber);
					}

					resized();
					return;
				}
			}
		}
	}
}

void JAdvancedDock::configureTabs(const TabDockType& vector) const
{
	int x = 0;

	auto lastComponent = getChildComponent(getNumChildComponents() - 1);

	for (auto& dockedCompWrapper : vector)
	{
		if (dockedCompWrapper->isVisible())
		{
			dockedCompWrapper->setShowTabButton(true, x, dockedCompWrapper.get() == lastComponent);
			x += dockedCompWrapper->getTabWidth() + 2;
		}
	}
}

void JAdvancedDock::resized()
{
	if (windows.size() == 0)
		return;

	auto area = getLocalBounds();

	auto vheight = area.getHeight() / windows.size();

	for (auto& row : windows)
	{
		if (row.size() == 0)
			continue; // shouldn't happen, but just for safety...

		auto area2 = area.removeFromTop(vheight);
		auto hwidth = area.getWidth() / row.size();

		for (auto& column : row)
		{
			auto area3 = area2.removeFromLeft(hwidth);

			for (auto& tabbedComponent : column)
				tabbedComponent->setBounds(area3);
		}
	}
}

void JAdvancedDock::paint(Graphics& g)
{
	if (windows.size() == 0)
	{
		g.fillAll(Colours::darkred);
		g.setColour(Colours::white);
		g.drawText("Drag a window here", getLocalBounds(), Justification::centred, false);
	}
}
