package mybot;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import core.Aim;
import core.Ants;
import core.Ilk;
import core.Tile;

public class MapTile extends Tile {

	private Ilk value = Ilk.UNKNOWN;
	private int lastSeen = 0;
	public static final int UNSET = Integer.MAX_VALUE;

	private HashMap<MapTile, Integer> realDistances = new HashMap<MapTile, Integer>();
	private float potential = 1;

	private Ant ant;
	private Food food;

	public MapTile(int row, int col) {
		super(row, col);
	}

	public void updateState() {
		if (isVisible()) {
			if (value.equals(Ilk.UNKNOWN))
				setValue(GameState.getCore().getIlk(this));
			lastSeen = GameState.getRound();
		}
	}

	public void setValue(Ilk value) {
		this.value = value;
	}

	public Ilk getValue() {
		return value;
	}

	public int getUnseenDuration() {
		return GameState.getRound() - lastSeen;
	}

	public static int CalculateHash(int row, int col) {
		return row * Ants.MAX_MAP_SIZE + col;
	}

	public boolean isVisible() {
		return GameState.getCore().isVisible(this);
	}

	public boolean isPassable() {
		return value != Ilk.UNKNOWN && value != Ilk.WATER;
	}

	public List<MapTile> getPassableNeighbours() {
		ArrayList<MapTile> tiles = new ArrayList<MapTile>();
		for (Aim direction : Aim.values()) {
			MapTile tile = getNeighbour(direction);
			if (tile.isPassable())
				tiles.add(tile);
		}
		return tiles;
	}

	public MapTile getPassableNeighbour(Aim aim) {
		MapTile tile = getNeighbour(aim);
		if (tile.isPassable())
			return tile;
		return null;
	}

	public MapTile getNeighbour(Aim aim) {
		int row = getRow() + aim.getRowDelta();
		int col = getCol() + aim.getColDelta();
		return GameState.getMap().getTile(row, col);
	}

	public void setAnt(Ant ant) {
		this.ant = ant;
	}

	public void unsetAnt() {
		this.ant = null;
	}

	public Ant getAnt() {
		return ant;
	}

	public boolean isOccupied() {
		return ant != null;
	}

	public boolean containFood() {
		return food != null;
	}

	public void setFood(Food food) {
		this.food = food;
	}

	public void unsetFood() {
		setFood(null);
	}

	public Food getFood() {
		return food;
	}

	public float getPotential() {
		return potential;
	}

	public void setPotential(float potential) {
		this.potential = potential;
	}

	public void decreasePotential(float amount) {
		this.potential -= amount;
	}

	private int cachedHashCode = UNSET;

	@Override
	public int hashCode() {
		if (cachedHashCode != UNSET)
			cachedHashCode = super.hashCode();
		return cachedHashCode;
	}

	public int getRealDistance(MapTile start) {
		if (start.equals(this))
			return 0;
		Integer distance = realDistances.get(start);
		if (distance == null)
			distance = start.realDistances.get(this);
		if (distance == null)
			return UNSET;
		return distance.intValue();
	}

	public void setRealDistance(MapTile destination, int distance) {
		realDistances.put(destination, distance);
	}

	public static boolean isValidTileWithAnt(MapTile maptile) {
		return maptile != null && maptile.getAnt() != null;
	}

	public String getValueCode() {
		if (containFood())
			return "f";
		if (isOccupied())
			return "a";
		if (value == Ilk.WATER)
			return "x";
		if (value == Ilk.LAND)
			return " ";
		return "#";
	}

	@Override
	public String toString() {
		return "Tile [" + getRow() + ", " + getCol() + "]";
	}

	public Tile createTile() {
		return new Tile(getRow(), getCol());
	}

	public Aim getAimTo(MapTile tile) {
		return GameState.getCore().getDirections(this, tile).get(0);
	}

}
