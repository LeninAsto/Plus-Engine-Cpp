#include "State.h"

#include <iostream>

std::optional<std::string> State::pendingStateChange_ = std::nullopt;

State::State(std::string name)
	: name_(std::move(name))
{
}

State::~State()
{
	ensureDestroyed();
}

void State::create()
{
	log("create()");
}

void State::update(float deltaSeconds)
{
	log("update(dt=" + std::to_string(deltaSeconds) + ")");
}

void State::render()
{
	log("render()");
}

void State::destroy()
{
	log("destroy()");
}

void State::onEnter()
{
	log("onEnter()");
}

void State::onExit()
{
	log("onExit()");
}

void State::ensureCreated()
{
	if (!created_) {
		created_ = true;
		create();
	}
}

void State::ensureDestroyed()
{
	if (created_) {
		destroy();
		created_ = false;
	}
}

const std::string& State::getName() const
{
	return name_;
}

bool State::isActive() const
{
	return active_;
}

bool State::isVisible() const
{
	return visible_;
}

bool State::isCreated() const
{
	return created_;
}

bool State::isPersistentUpdate() const
{
	return persistentUpdate_;
}

bool State::isPersistentDraw() const
{
	return persistentDraw_;
}

void State::setActive(bool active)
{
	active_ = active;
}

void State::setVisible(bool visible)
{
	visible_ = visible;
}

void State::setPersistentUpdate(bool persistentUpdate)
{
	persistentUpdate_ = persistentUpdate;
}

void State::setPersistentDraw(bool persistentDraw)
{
	persistentDraw_ = persistentDraw;
}

void State::requestStateChange(const std::string& stateName)
{
	pendingStateChange_ = stateName;
}

bool State::hasPendingStateChange()
{
	return pendingStateChange_.has_value();
}

std::string State::consumePendingStateChange()
{
	if (!pendingStateChange_.has_value()) {
		return {};
	}

	const std::string stateName = *pendingStateChange_;
	pendingStateChange_.reset();
	return stateName;
}

void State::log(const std::string& message) const
{
	std::cout << "[State: " << name_ << "] " << message << '\n';
}
