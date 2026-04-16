#pragma once

#include <optional>
#include <string>

class State
{
public:
	explicit State(std::string name = "State");
	virtual ~State();

	virtual void create();
	virtual void update(float deltaSeconds);
	virtual void render();
	virtual void destroy();

	virtual void onEnter();
	virtual void onExit();

	void ensureCreated();
	void ensureDestroyed();

	const std::string& getName() const;
	bool isActive() const;
	bool isVisible() const;
	bool isCreated() const;
	bool isPersistentUpdate() const;
	bool isPersistentDraw() const;

	void setActive(bool active);
	void setVisible(bool visible);
	void setPersistentUpdate(bool persistentUpdate);
	void setPersistentDraw(bool persistentDraw);
	static void requestStateChange(const std::string& stateName);
	static bool hasPendingStateChange();
	static std::string consumePendingStateChange();

protected:
	void log(const std::string& message) const;

private:
	static std::optional<std::string> pendingStateChange_;
	std::string name_;
	bool active_ = true;
	bool visible_ = true;
	bool created_ = false;
	bool persistentUpdate_ = false;
	bool persistentDraw_ = true;
};
