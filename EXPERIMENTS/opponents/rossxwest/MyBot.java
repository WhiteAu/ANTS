import java.io.IOException;
import java.util.*;

/**
 * Starter bot implementation.
 */
public class MyBot extends Bot {
    /**
     * Main method executed by the game engine for starting the bot.
     * 
     * @param args command line arguments
     * 
     * @throws IOException if an I/O error occurs
     */
    private AMap map = new AMap();

    public static void main(String[] args) throws IOException {
        new MyBot().readSystemInput();
    }

    @Override
    public void doTurn() {
        Ants ants = getAnts();

        map.setAnts(ants);
        map.scoreMap();        
        map.doMoves();
    }
}