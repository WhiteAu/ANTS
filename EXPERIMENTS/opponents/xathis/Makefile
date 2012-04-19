JAVAC=javac
JAVAC_ARGS=

SOURCES=*.java
CLASSES=$(addsuffix .class, $(basename ${SOURCES}))
JAR=xathis.jar

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