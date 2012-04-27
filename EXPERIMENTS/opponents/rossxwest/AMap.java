import java.util.*;
import java.io.*;


public class AMap {
    
    private Ants ants;
    
    private double scoredMap[][];

    private Map<Tile, Tile> orders = new HashMap<Tile, Tile>();
    private int enemyMap[][];
    private int friendlyMap[][];

    private int rows, cols;
    private int turn = 0;
    private int exploreMap[][];
    private int diffuse;

    private Set<Ant> myAnts;
    private Set<Ant> enemyAnts;
    private Set<Tile> enemyHills;
    private Set<Tile> food;
    private Set<Tile> myHills;

    private ArrayList<Tile> dEnemyHills = new ArrayList<Tile>();
    private ArrayList<Tile> foodCache = new ArrayList<Tile>();

    // Begin Weights

    private double ehWeight = 200;

    private double eaEnemyRadius = 0;
    private double eaFriendlyRadius = 0;
    private double eaDistanceToHillRadius = 100;
    private double eaOutnumberWeight = 0;
    private double eaOutnumberedWeight = 0;
    private double eaEqualWeight = 0;
    private double eaWeight = 2;

    private double fFriendlyRadius = 0;
    private double fFriendlyWeight = 0;
    private double fEnemyRadius = 0;
    private double fEnemyWeight = 0;
    private double fWeight = 25;

    private double eWeight = .25;

    double dCoefficient = .33;

    // End Weights

    public AMap() {

    }

    public void setAnts(Ants a) {

        int i, j;

        if (ants == null) {
        
            ants = a;
            rows = ants.getRows();
            cols = ants.getCols();
            
            diffuse = 150;

            scoredMap = new double[rows][cols];
            exploreMap = new int[rows][cols];
            enemyMap = new int[rows][cols];
            friendlyMap = new int[rows][cols];

        }

    }

    public Aim directionTo(Tile from, Tile to) {

        int fR = from.getRow();
        int fC = from.getCol();
        int tR = from.getRow();
        int tC = from.getCol();

        int distance = ants.getDistance(from, to);

        int rDis = (Math.abs(fR - tR) < distance) ? Math.abs(fR - tR) : Math.abs((rows - fR) - tR);
        int cDis = (Math.abs(fC - tC) < distance) ? Math.abs(fC - tC) : Math.abs((rows - fC) - tC);

        if (rDis >= cDis) {
            if (fR < tR) return Aim.NORTH;
            return Aim.SOUTH;
        } else {
            if (fC < tC) return Aim.WEST;
            return Aim.EAST;
        }

    }

    public void scoreMap() {

        myAnts = ants.getMyAnts();
        enemyAnts = ants.getEnemyAnts();
        enemyHills = ants.getEnemyHills();
        food = ants.getFoodTiles();
        myHills = ants.getMyHills();

        turn++;

        orders.clear();

        int i, j;

        for (i = 0; i < rows; i++) {
            for(j = 0; j < cols; j++) {
                scoredMap[i][j] = 0;
            }
        }

        scoreFood();
        scoreExplore();
        scoreEnemyHills();
        scoreEnemyAnts();
        diffuseMap();
        scoreCombat();

        for (Tile t : myHills) {
            Tile ne = ants.getTile(t, Aim.NORTHEAST);
            Tile nw = ants.getTile(t, Aim.NORTHWEST);
            Tile se = ants.getTile(t, Aim.SOUTHEAST);
            Tile sw = ants.getTile(t, Aim.SOUTHWEST);

            scoredMap[t.getRow()][t.getCol()] = -1;
            
            if (myAnts.size() / myHills.size() > 10) {
                if (turn > 25) {
                    scoredMap[ne.getRow()][ne.getCol()] = 100;
                }
                
                if (turn > 50) {
                    scoredMap[nw.getRow()][nw.getCol()] = 100;
                }
                
                if (turn > 75) {
                    scoredMap[se.getRow()][se.getCol()] = 100;
                }
                
                if (turn > 100) {
                    scoredMap[sw.getRow()][sw.getCol()] = 100;
                }
            }
 
        }

    }

    private void scoreEnemyHills() {

        ArrayList<Tile> notfound = new ArrayList<Tile>();

        for (Tile t : dEnemyHills) {
            boolean found = false;
            if (ants.isVisible(t)) {

                if (enemyHills.contains(t)) found = true;

                if (!found) {
                    notfound.add(t);
                }

            }

        }

        for (Tile t : notfound) {
            dEnemyHills.remove(t);
        }
       
        for (Tile t : enemyHills) {
            if (!dEnemyHills.contains(t)) dEnemyHills.add(t);           
        }

        for (Tile t : dEnemyHills) {
            scoredMap[t.getRow()][t.getCol()] += ehWeight;
        }

    }

    private void scoreEnemyAnts() {

        for (Ant ea : enemyAnts) {
            Tile t = ea.getTile();
            int disToHill = 100000;
            int numEnemy = 0;
            int numFriendly = 0;
            int numEnemyClustered = 1;
            int numFriendlyClustered = 1;

            for (Tile h : myHills) {
                    disToHill = Math.min(disToHill, ants.getDistance(h, t));
            }
            
            for (Ant ea2 : enemyAnts) {
                Tile e = ea2.getTile();
                if (ants.getDistance(e, t) < Math.sqrt(ants.getAttackRadius2())) {
                    numEnemy++;
                }
            }
                 
            for (Ant fa : myAnts) {
                Tile f = fa.getTile();
                if (ants.getDistance(f, t) < Math.sqrt(ants.getAttackRadius2()) + 2) {
                    numFriendly++;
                }
            }

            if (disToHill <= eaDistanceToHillRadius) {
                scoredMap[t.getRow()][t.getCol()] += (eaDistanceToHillRadius - disToHill) / 4;
            } 

            if (numEnemy > numFriendly) {
                scoredMap[t.getRow()][t.getCol()] += eaOutnumberedWeight;
            }

            if (numEnemy == numFriendly) {
                scoredMap[t.getRow()][t.getCol()] += eaEqualWeight;
            }

            if (numEnemy < numFriendly) {
                scoredMap[t.getRow()][t.getCol()] += eaOutnumberWeight;
            }
            
            scoredMap[t.getRow()][t.getCol()] += eaWeight;

        }

    }

    private void scoreCombat() {

        int ar2 = ants.getAttackRadius2();
        int mr = (int)Math.sqrt(ar2) + 1;
        int i, j;

        enemyMap = new int[rows][cols];
        friendlyMap = new int[rows][cols];

        for (Ant ea : enemyAnts) {

            if (ants.getTimeRemaining() < 40) return;

            Tile e = ea.getTile();

            int r = e.getRow();
            int c = e.getCol();

            for (i = -mr; i <= mr; i++) {
                for (j = -mr; j <= mr; j++) {

                    int row = (r + i) % rows;
                    if (row < 0) {
                        row += rows;
                    }

                    int col = (c + j) % cols;
                    if (col < 0) {
                        col += cols;
                    }

                    if (Math.abs(i) + Math.abs(j) < ar2) {
                        enemyMap[row][col]++;                        
                    }
                }
            }

        }

        for (Ant ma : myAnts) {
            
            if (ants.getTimeRemaining() < 40) return;

            Tile m = ma.getTile();

            int r = m.getRow();
            int c = m.getCol();

            for (i = -mr; i <= mr; i++) {
                for (j = -mr; j <= mr; j++) {

                    int row = (r + i) % rows;
                    if (row < 0) {
                        row += rows;
                    }

                    int col = (c + j) % cols;
                    if (col < 0) {
                        col += cols;
                    }

                    if (Math.abs(i) + Math.abs(j) < ar2) {
                        friendlyMap[row][col]++;
                    }
                }
            }

        }
        
    }

    private void scoreFood() {

        ArrayList<Tile> mFood = new ArrayList<Tile>();

        for (Tile t : foodCache) {
            if (ants.isVisible(t) && !food.contains(t)) {
                mFood.add(t);
            }
        }

        for (Tile t : mFood) {
            foodCache.remove(t);
        }

        for (Tile t : food) {
            if (!foodCache.contains(t)) {
                foodCache.add(t);
            }
        }        

        for (Tile t : foodCache) {
            scoredMap[t.getRow()][t.getCol()] += fWeight;
        }

    }

    private void scoreExplore() {
        
        int i, j;

        for (i = 0; i < rows; i++) {
            for(j = 0; j < cols; j++) {
                Tile ex = new Tile(i,j);
                if (!ants.isVisible(ex) && ants.getIlk(ex).isPassable()) {
                    exploreMap[i][j]++;
                    if (exploreMap[i][j] > 10) scoredMap[i][j] += eWeight * exploreMap[i][j];
                } else if (!ants.getIlk(ex).isPassable() || ants.isVisible(ex)) { 
                    exploreMap[i][j] = 0;
                }
            }
        }        

    }

    private void diffuseMap() {

        int i, j, k;
        double tempMap[][] = new double[rows][cols];

        for (k = 0; k < diffuse; k++) {

            if (ants.getTimeRemaining() < 40) break;

            tempMap = new double[rows][cols];

            for (i = 0; i < rows; i++) {
                for (j = 0; j < cols; j++) {

                    Tile c = new Tile(i, j);

                    if (ants.getIlk(c).isPassable()) {

                        Tile n = ants.getTile(c, Aim.NORTH);
                        Tile s = ants.getTile(c, Aim.SOUTH);
                        Tile e = ants.getTile(c, Aim.EAST);
                        Tile w = ants.getTile(c, Aim.WEST);

                        int nR = n.getRow();
                        int nC = n.getCol();
                        int sR = s.getRow();
                        int sC = s.getCol();
                        int eR = e.getRow();
                        int eC = e.getCol();
                        int wR = w.getRow();
                        int wC = w.getCol();

                        double value = scoredMap[nR][nC] + scoredMap[sR][sC] + scoredMap[eR][eC] + scoredMap[wR][wC];
                        //double dVal = diffuse * dCoefficient;
                        double dVal = 4;

                        if (scoredMap[i][j] >= 0 && value <= 0) {
                            tempMap[i][j] = scoredMap[i][j] + (value / dVal);
                        } else if (scoredMap[i][j] <= 0 && value >= 0) {
                            tempMap[i][j] = scoredMap[i][j] + (value / dVal);
                        } else if (scoredMap[i][j] <= 0 && value <= 0) {
                            tempMap[i][j] = Math.min(value / dVal, scoredMap[i][j]);
                        } else {
                            tempMap[i][j] = Math.max(value / dVal, scoredMap[i][j]);
                        }

                        if (myAnts.contains(c)) {
                            tempMap[i][j] /= 2;
                        }

                    }

                }
            }

            scoredMap = tempMap;

        }

    }

    private double scoreTile(int r, int c, boolean here) {

        Tile t = new Tile(r, c);

        if (!ants.getIlk(t).isUnoccupied() && orders.containsValue(t) && !orders.containsKey(t) || here) {
            if (enemyMap[r][c] > 0) {
                if (friendlyMap[r][c] > enemyMap[r][c]) {
                    return 25 + friendlyMap[r][c] - enemyMap[r][c];
                } else {
                    return -enemyMap[r][c];
                }
            }      
            return scoredMap[r][c];
        } else if (!ants.getIlk(t).isPassable()) {
            return -1000000;
        } else if (!ants.getIlk(t).isUnoccupied() || orders.containsKey(t)) {
            return -1000000;
        }

        if (enemyMap[r][c] > 0) {
            if (friendlyMap[r][c] > enemyMap[r][c]) {
                return 25 + friendlyMap[r][c] - enemyMap[r][c];
            } else {
                return -enemyMap[r][c];
            }
        }      
        
        return scoredMap[r][c];

    }

    public void doMove(Tile from) {

        int r = from.getRow();
        int c = from.getCol();
        double best; 
        int bestR, bestC;
        int nR, nC, sR, sC, eR, eC, wR, wC;

        List<Aim> dir;

        nR = (r - 1 < 0) ? rows - 1 : r - 1;
        nC = c;
        sR = (r + 1 >= rows) ? 0 : r + 1;
        sC = c;
        eR = r;
        eC = (c + 1 >= cols) ? 0 : c + 1;
        wR = r;
        wC = (c - 1 < 0) ? cols - 1 : c - 1;

        double north = scoreTile(nR, nC, false);
        if (ants.getTimeRemaining() < 40) return;
        double south = scoreTile(sR, sC, false);
        if (ants.getTimeRemaining() < 40) return;
        double east = scoreTile(eR, eC, false);
        if (ants.getTimeRemaining() < 40) return;
        double west = scoreTile(wR, wC, false);
        if (ants.getTimeRemaining() < 40) return;
        double here = scoreTile(r, c, true);
        if (ants.getTimeRemaining() < 40) return;

        best = Math.max(north, Math.max(south, Math.max(east, Math.max(west, here))));

        if (best > -1000000) {

            if (best == north) {
                bestR = nR;
                bestC = nC;
            } else if (best == south) {
                bestR = sR;
                bestC = sC;
            } else if (best == east) {
                bestR = eR;
                bestC = eC;
            } else if (best == west) {
                bestR = wR;
                bestC = wC;
            } else {
                return;
            }

            dir = ants.getDirections(from, new Tile(bestR, bestC));
            
            ants.issueOrder(from, dir.get(0));
            orders.put(ants.getTile(from, dir.get(0)), from);

            if (ants.getTimeRemaining() < 40) return;

        }

    }

    public void doMoves() {

        for (Ant a : myAnts) {
            Tile t = a.getTile();
            if (ants.getTimeRemaining() < 40) break;
            doMove(t);
        }

    }

}
