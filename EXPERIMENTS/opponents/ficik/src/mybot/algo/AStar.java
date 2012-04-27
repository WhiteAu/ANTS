package mybot.algo;

import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.PriorityQueue;
import java.util.Set;
import mybot.Ant;
import mybot.GameState;
import mybot.Map;
import mybot.MapTile;

public class AStar {

	private static final int DISTANCE_CUTOFF = 15;
	private Set<MapTile> closedSet = new HashSet<MapTile>();
	private MapTile start;
	private List<? extends Goal> goals;
	private Set<MapTile> closedGoalTiles = new HashSet<MapTile>();
	private PriorityQueue<MapTile> openSet = new PriorityQueue<MapTile>(80, new FValueComparator());
	private boolean cutoffFlag = false;

	public static Iterator<MapTile> antSearch(MapTile start) {
		return (new AStar(start, Ant.getAnts())).getIterator();
	}

	private AStar(MapTile start, List<? extends Goal> goals) {
		this.start = start;
		this.goals = goals;
		for (Goal goal : goals) {
			if (h(start, goal.getMapTile()) > DISTANCE_CUTOFF) {
				closedGoalTiles.add(goal.getMapTile());
			}
		}
		openSet.add(start);
	}

	private boolean checkIfGoal(MapTile tile) {
		for (Goal goal : goals) {
			if (goal.getMapTile().equals(tile))
				return true;
		}
		return false;
	}

	private void recalculateOpenListPriority() {
		PriorityQueue<MapTile> newOpenSet = new PriorityQueue<MapTile>(80, new FValueComparator());
		while (!openSet.isEmpty()) {
			newOpenSet.add(openSet.poll());
		}
		openSet = newOpenSet;
	}

	public Iterator<MapTile> getIterator() {
		return new AStarIterator(this);
	}

	private MapTile search() {
		while (!openSet.isEmpty()) {
			MapTile processed = openSet.poll();
			closedSet.add(processed);
			if (checkIfGoal(processed)) {
				closedGoalTiles.add(processed);
				return processed;
			}
			int curDistance = processed.getRealDistance(start);
			if (curDistance > DISTANCE_CUTOFF) {
				cutoffFlag = true;
				break;
			}
			for (MapTile tile : processed.getPassableNeighbours())
				tryAddToOpenList(tile, curDistance + 1);
		}
		return null;
	}

	private void tryAddToOpenList(MapTile tile, int distance) {
		if (closedSet.contains(tile) || openSet.contains(tile))
			return;
		tile.setRealDistance(start, distance);
		openSet.add(tile);
	}

	private HashMap<MapTile, Integer> fCache = new HashMap<MapTile, Integer>();

	private int f(MapTile maptile) {
		Integer value = fCache.get(maptile);
		if (value == null) {
			value = g(maptile) + minH(maptile);
			fCache.put(maptile, value);
		}
		return value;
	}

	private int g(MapTile maptile) {
		return start.getRealDistance(maptile);
	}

	private int h(MapTile pos, MapTile goal) {
		int dist = pos.getRealDistance(goal);
		if (dist != MapTile.UNSET)
			return dist;
		return Map.getManhattanDistance(pos, goal);
	}

	private int minH(MapTile pos) {
		int best = Integer.MAX_VALUE;
		Iterator<? extends Goal> iterator = goals.iterator();
		while (iterator.hasNext()) {
			MapTile goal = iterator.next().getMapTile();
			if (closedGoalTiles.contains(goal))
				continue;
			best = Math.min(h(pos, goal), best);
		}
		return best;
	}

	private boolean isCutoffed() {
		return cutoffFlag;
	}

	private class AStarIterator implements Iterator<MapTile> {

		private AStar algo;

		public AStarIterator(AStar algo) {
			this.algo = algo;
		}

		@Override
		public boolean hasNext() {
			return !openSet.isEmpty() && (goals.size() - closedGoalTiles.size()) > 0 && !algo.isCutoffed();
		}

		@Override
		public MapTile next() {
			algo.recalculateOpenListPriority();
			return algo.search();
		}

		@Override
		public void remove() {
			// It's lazy generated, nothing to remove
		}

	}

	private class FValueComparator implements Comparator<MapTile> {
		@Override
		public int compare(MapTile o1, MapTile o2) {
			return ((Integer) f(o1)).compareTo((Integer) f(o2));
		}
	}
}
