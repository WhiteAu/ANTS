package mybot;

import core.Ants;

public class GameState {

	private static GameState instance;
	private static int round = 0;
	private static Map map;
	private static Ants core;
	private boolean logging = true;
	private GameLogger logger = new GameLogger();

	/* ********************** */
	/* * SINGLETON SECTION */
	/* ********************** */

	public static GameState getInstance() {
		if (instance == null)
			instance = new GameState();
		return instance;
	}

	public static void setMap(Map map) {
		GameState.map = map;
	}

	public static Map getMap() {
		return GameState.map;
	}

	public static void setCore(Ants core) {
		GameState.core = core;
	}

	public static Ants getCore() {
		return GameState.core;
	}

	public static GameLogger getLogger() {
		return getInstance().logger;
	}

	public static void log(String message) {
		getInstance().logger.log(message);
	}

	/* ************************ */
	/* END OF SINGLETON SECTION */
	/* ************************ */

	public static int getRound() {
		return round;
	}

	public void prepareNewRound() {
		round += 1;
		logRoundChanges();
		GameState.map.updateWholeMap();
	}

	public boolean isLogging() {
		return logging;
	}

	public void logRoundChanges() {
		if (isLogging())
			logger.logRoundChanges();
	}
}
