JAVAC=javac
JAVAC_ARGS=

SOURCES=Tile.java Ant.java Ants.java Bot.java Aim.java Ilk.java Order.java AMap.java AbstractSystemInputParser.java AbstractSystemInputReader.java MyBot.java
CLASSES=$(addsuffix .class, $(basename ${SOURCES}))
JAR=MyBot.jar

.PHONY: all clean

all: $(CLASSES) $(JAR)

$(JAR): $(CLASSES)
	jar cvfm $(JAR) Manifest.txt *.class

%.class: %.java
	$(JAVAC) $(JAVAC_ARGS) $<

clean:
	-rm -Rf $(CLASSES)
	-rm -Rf *.class
	-rm -Rf $(JAR)