package edu.duke.raft;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface RaftServer extends Remote {

  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0 if server votes for candidate under candidate's term; 
  // otherwise, return server's current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) 
    throws RemoteException;

  // @return 0 if server appended entries under the leader's term; 
  // otherwise, return server's current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) 
    throws RemoteException;
}
