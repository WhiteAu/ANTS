import java.io.IOException;
import mybot.Ant;
import mybot.Food;
import mybot.GameState;
import mybot.Map;
import mybot.algo.DCOP;
import mybot.algo.PotentialFields;
import core.Bot;
import core.Ilk;

/**
 * Starter bot implementation.
 */
public class MyBot extends Bot {

	public static final int MY_BOT = 0;

	public static void main(String[] args) throws IOException {
		MyBot bot = new MyBot();
		bot.readSystemInput();
	}

	public void prepare() {
		GameState.setCore(getAnts());
		GameState.setMap(new Map());
		DCOP.initGameSpecificVariables();
	}

	@Override
	public void doTurn() {
		if (GameState.getRound() == 0)
			prepare();

		GameState.getInstance().prepareNewRound();

		/*
		 * for (Tile hill : getAnts().getEnemyHills()){ MapTile closest =
		 * AStar.antSearch(GameState.getMap().getTile(hill.getRow(),
		 * hill.getCol())); if (isValidTileWithAnt(closest))
		 * closest.getAnt().assignHill(hill); }
		 */
		Food.checkAndRemoveLostFood();
		Food.tryAssignFood();
		Ant.scheduleMoves();
		Ant.performMoves();

		GameState.getLogger().logMap();
	}

	/* *********** *
	 * LISTENERS * ***********
	 */

	@Override
	public void addFood(int row, int col) {
		Food.addFood(row, col);
		super.addFood(row, col);
	}

	@Override
	public void addAnt(int row, int col, int owner) {
		if (owner == MY_BOT)
			Ant.addAnt(row, col);
		super.addAnt(row, col, owner);
	}

	@Override
	public void removeAnt(int row, int col, int owner) {
		if (owner == MY_BOT)
			Ant.removeAnt(row, col);
		super.removeAnt(row, col, owner);
	}

	@Override
	public void addWater(int row, int col) {
		GameState.getMap().getTile(row, col).setValue(Ilk.WATER);
		PotentialFields.generatePotentialFromTile(row, col);
	}
}
