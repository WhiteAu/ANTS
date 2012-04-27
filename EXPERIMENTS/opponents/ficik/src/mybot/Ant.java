package mybot;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import mybot.algo.DCOP;
import mybot.algo.Goal;
import core.Aim;

public class Ant implements Goal {

	/* ====== Ant manager part ======== */

	private static ArrayList<Ant> ants = new ArrayList<Ant>();

	public static void addAnt(int row, int col) {
		if (!GameState.getMap().getTile(row, col).isOccupied())
			new Ant(row, col);
	}

	public static void removeAnt(int row, int col) {
		Ant ant = GameState.getMap().getTile(row, col).getAnt();
		if (ant != null) {
			ants.remove(ant);
			ant.unsetPosition();
		}
	}

	public static void scheduleMoves() {
		for (Ant ant : ants) {
			ant.scheduleMove();
		}
	}

	public static void performMoves() {
		for (Ant ant : ants) {
			ant.performScheduledMove();
		}
	}

	public static List<Ant> getAnts() {
		return ants;
	}

	/* === Single ant specific part === */

	private MapTile maptile = null;
	private List<MapTile> scheduledMoves = new ArrayList<MapTile>();
	private Target assignedTarget;

	private Ant(int row, int col) {
		setPosition(row, col);
		ants.add(this);
	}

	public void scheduleMove() {
		scheduledMoves.clear();
		if (hasAssignedTarget())
			scheduleMoveToTarget();
		scheduleMoveToFight();
		if (scheduledMoves.isEmpty())
			scheduleMoveToDiscover();
	}

	private void scheduleMoveToTarget() {
		scheduledMoves = maptile.getPassableNeighbours();
		Collections.sort(scheduledMoves, new Comparator<MapTile>() {
			@Override
			public int compare(MapTile o1, MapTile o2) {
				return ((Integer) ((int) (o1.getRealDistance(assignedTarget.getMapTile())))).compareTo((int) (o2
						.getRealDistance(assignedTarget.getMapTile())));
			}
		});
	}

	private void scheduleMoveToFight() {

	}

	private void scheduleMoveToDiscover() {
		scheduledMoves = DCOP.getNeighbourMapTilesOrderedByExploringValue(maptile);
	}

	public void performScheduledMove() {
		for (MapTile tile : scheduledMoves) {
			if (tryMoveToMapTile(tile))
				break;
		}
	}

	private void unsetPosition() {
		if (maptile == null)
			return;
		maptile.setAnt(null);
		maptile = null;
	}

	private void setPosition(int row, int col) {
		setPosition(GameState.getMap().getTile(row, col));
	}

	private void setPosition(MapTile maptile) {
		unsetPosition();
		this.maptile = maptile;
		maptile.setAnt(this);
	}

	public MapTile getMapTileInDirection(Aim aim) {
		return maptile.getPassableNeighbour(aim);
	}

	public boolean tryMoveToMapTile(MapTile maptile) {
		if (maptile == null || maptile.isOccupied())
			return false;
		issueMoveOrder(maptile);
		setPosition(maptile);
		return true;
	}

	private void issueMoveOrder(MapTile destination) {
		Aim direction = GameState.getCore().getDirections(maptile, destination).get(0);
		GameState.getCore().issueOrder(maptile, direction);
	}

	@Override
	public MapTile getMapTile() {
		return maptile;
	}

	public boolean assignTargetIfBetter(Target target) {
		if (isNewTargetIsBetter(target)) {
			assignTarget(target);
			return true;
		}
		return false;
	}

	public boolean hasAssignedTarget() {
		return (assignedTarget != null && assignedTarget.targetExists());
	}

	private boolean isNewTargetIsBetter(Target target) {
		if (!hasAssignedTarget())
			return true;
		int newDistance = target.getRealDistance(maptile);
		int oldDistance = assignedTarget.getRealDistance(maptile);
		return (newDistance != MapTile.UNSET && newDistance < oldDistance);
	}

	private void assignTarget(Target target) {
		if (hasAssignedTarget())
			assignedTarget.unassign();
		assignedTarget = target;
		assignedTarget.assign();
	}

	@Override
	public String toString() {
		return "Ant at " + maptile + ": Target: " + assignedTarget + " ["
				+ (hasAssignedTarget() ? "exists" : "dont exists") + "]";
	}

}
