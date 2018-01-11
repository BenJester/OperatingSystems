RAFT_PATH=edu/duke/raft
SRC_DIR=src/$(RAFT_PATH)
JC = javac
JFLAGS = -d ./bin -classpath ./bin:./src

.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = \
	$(SRC_DIR)/CandidateMode.java \
	$(SRC_DIR)/Entry.java \
	$(SRC_DIR)/FollowerMode.java \
	$(SRC_DIR)/LeaderMode.java \
	$(SRC_DIR)/RaftConfig.java \
	$(SRC_DIR)/RaftLog.java \
	$(SRC_DIR)/RaftMode.java \
	$(SRC_DIR)/RaftResponses.java \
	$(SRC_DIR)/RaftServer.java \
	$(SRC_DIR)/RaftServerImpl.java \
	$(SRC_DIR)/StartClient.java \
	$(SRC_DIR)/StartServer.java

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) ./bin/$(RAFT_PATH)/*.class
