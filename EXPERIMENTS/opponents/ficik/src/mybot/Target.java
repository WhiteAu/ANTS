package mybot;

import mybot.algo.Goal;

abstract public class Target implements Goal {

	private int assignees = 0;
	private boolean exists = true;

	public boolean isAssigned() {
		return assignees > 0;
	}

	public void assign() {
		assignees += 1;
	}

	public void unassign() {
		if (assignees != 0)
			assignees -= 1;
	}

	public boolean targetExists() {
		return exists;
	}

	protected void destroy() {
		exists = false;
	}

	@Override
	abstract public MapTile getMapTile();

	public int getRealDistance(MapTile maptile) {
		return maptile.getRealDistance(getMapTile());
	}
}
