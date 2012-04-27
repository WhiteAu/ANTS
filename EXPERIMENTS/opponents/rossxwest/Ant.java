/**
 * Represents a tile of the game map.
 */
public class Ant extends Tile {
    
    private int owner;

    /**
     * Creates new {@link Tile} object.
     * 
     * @param row row index
     * @param col column index
     */
    public Ant(int r, int c, int o) {
        super(r, c);
        owner = o;
    }
    
    public int getOwner() {
        return owner;
    }

    public Tile getTile() {
        return new Tile(getRow(), getCol());
    }
    
    @Override
    public String toString() {
        return getRow() + " " + getCol() + " " + owner;
    }
}
