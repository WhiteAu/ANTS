JAVAC=javac
JAVAC_ARGS=-d ../bin/

SOURCES=Ants.java Bot.java Aim.java Ilk.java Order.java Tile.java AbstractSystemInputParser.java AbstractSystemInputReader.java MyBot.java
CLASSES=$(addsuffix .class, $(basename ${SOURCES}))
JAR=../dist/MyBot.jar

.PHONY: all clean

all: $(CLASSES) $(JAR)

$(JAR): $(CLASSES)
	jar cvfm $(JAR) Manifest.txt ../bin/*.class

%.class: %.java
	$(JAVAC) $(JAVAC_ARGS) $<

clean:
	-rm -Rf $(CLASSES)
	-rm -Rf *.class
	-rm -Rf $(JAR)
