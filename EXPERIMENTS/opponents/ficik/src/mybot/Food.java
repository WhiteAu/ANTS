package mybot;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import mybot.algo.AStar;
import mybot.algo.Goal;

public class Food extends Target implements Goal {

	/* ==== Food Manager ===== */

	private static List<Food> food = new ArrayList<Food>();

	public static void addFood(int row, int col) {
		if (!GameState.getMap().getTile(row, col).containFood())
			new Food(row, col);
	}

	public static void removeFood(int row, int col) {
		GameState.getMap().getTile(row, col).getFood().destroy();
	}

	public static void checkAndRemoveLostFood() {
		Queue<Food> markedToRemove = new LinkedList<Food>();
		for (Food the_food : food)
			if (!the_food.exists())
				markedToRemove.add(the_food);
		while (!markedToRemove.isEmpty())
			markedToRemove.poll().destroy();
	}

	public static List<Food> getFood() {
		return food;
	}

	public static void tryAssignFood() {
		for (Food the_food : food)
			if (!the_food.isAssigned())
				the_food.tryToAssignToClosestAnt();
	}

	/* ===== Food Instance ====== */

	private MapTile maptile = null;

	private Food(int row, int col) {
		maptile = GameState.getMap().getTile(row, col);
		maptile.setFood(this);
		food.add(this);
	}

	@Override
	protected void destroy() {
		maptile.unsetFood();
		food.remove(this);
		super.destroy();
	}

	@Override
	public MapTile getMapTile() {
		return maptile;
	}

	public boolean exists() {
		return !maptile.isVisible() || GameState.getCore().getFoodTiles().contains(maptile.createTile());
	}

	public void tryToAssignToClosestAnt() {
		GameState.log("Assigning " + this);
		Iterator<MapTile> closestIterator = AStar.antSearch(maptile);
		while (closestIterator.hasNext()) {
			MapTile closest = closestIterator.next();
			GameState.log("- Found " + closest);
			if (MapTile.isValidTileWithAnt(closest))
				if (closest.getAnt().assignTargetIfBetter(this)) {
					GameState.log("success");
					break;
				}
			GameState.log("fail");
		}
	}

	@Override
	public String toString() {
		return (isAssigned() ? "Assigned " : "") + "Food at " + maptile;
	}
}
