package edu.duke.raft;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class RaftServerImpl extends UnicastRemoteObject 
  implements RaftServer {

  private static int mID;
  private static RaftMode mMode;
  private static Object mLock;
  
  // @param server's unique id
  public RaftServerImpl (int serverID) throws RemoteException {
    mID = serverID;
  }

  // @param the server's current mode
  public static void setMode (RaftMode mode) {
    synchronized (mLock) { 
      if (mode == null) {
	return;
      }
      // only change to a new mode
      if ((mMode == null) || 
	  (mMode.getClass () != mode.getClass ())) {
	mMode = mode;
	mode.go ();    
      }
    }
  }
  
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
    throws RemoteException {
    synchronized (mLock) { 
      return mMode.requestVote (candidateTerm, 
				candidateID, 
				lastLogIndex,
				lastLogTerm);
    }
  }

  // @return 0 if server appended entries under the leader's term; 
  // otherwise, return server's current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) 
    throws RemoteException  {
    synchronized (mLock) { 
      return mMode.appendEntries (leaderTerm,
				  leaderID,
				  prevLogIndex,
				  prevLogTerm,
				  entries,
				  leaderCommit);
    }
  }

  static {
    mLock = new Object();
  }
}
