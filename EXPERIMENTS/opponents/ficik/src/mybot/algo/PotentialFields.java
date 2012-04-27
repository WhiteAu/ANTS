package mybot.algo;

import mybot.GameState;

public class PotentialFields {

	private static final float LEVEL_1 = 1f / 4;
	private static final float LEVEL_2 = 1f / 16;

	public static void generatePotentialFromTile(int row, int col) {
		GameState.getMap().getTile(row, col).setPotential(0);
		level1(row, col);
		level2(row, col);
	}

	/*
	 * x x0x x
	 */
	private static void level1(int row, int col) {
		decreasePotential(row + 1, col, LEVEL_1);
		decreasePotential(row - 1, col, LEVEL_1);
		decreasePotential(row, col + 1, LEVEL_1);
		decreasePotential(row, col - 1, LEVEL_1);
	}

	/*
	 * x x1x x101x x1x x
	 */
	private static void level2(int row, int col) {
		decreasePotential(row + 1, col + 1, 2 * LEVEL_2);
		decreasePotential(row - 1, col + 1, 2 * LEVEL_2);
		decreasePotential(row + 1, col - 1, 2 * LEVEL_2);
		decreasePotential(row - 1, col - 1, 2 * LEVEL_2);

		decreasePotential(row + 2, col, LEVEL_2);
		decreasePotential(row - 2, col, LEVEL_2);
		decreasePotential(row, col + 2, LEVEL_2);
		decreasePotential(row, col - 2, LEVEL_2);

	}

	private static void decreasePotential(int row, int col, float amount) {
		GameState.getMap().getTile(row, col).decreasePotential(amount);
	}
}
