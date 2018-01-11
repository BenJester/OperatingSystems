package edu.duke.raft;

/*
 * This class provides a centralized collection point for RPC
 * responses (i.e., requestVote and appendEntries). Responses are
 * collected under a given term. The term must be set to begin a
 * collection period, and incoming responses that do not match the
 * internal term will be ignored.
 *
 * Accesses to RaftResponses must be properly synchronized.
 */

public class RaftResponses {

  private static int[] mAppendResponses;
  private static int mTerm;
  private static int[] mVotes;
  private static int[] mRounds;

  // @param size of the network
  // @param current term
  public static void init (int size, int currentTerm) {
    mVotes = new int[size + 1];
    mRounds = new int[size + 1];
    mAppendResponses = new int[size + 1];    
    setTerm (currentTerm);
    clearVotes (currentTerm);
    clearAppendResponses (currentTerm);
  }

  // @param the current term
  public static void setTerm (int currentTerm) {
    mTerm = currentTerm;
    // restart the round count
    for (int i=0; i<mRounds.length; i++) {
      mRounds[i] = -1;
    }
  }

  // @param current term. 
  // @return null if the internal term not equal to the
  // paramter. array of voting results otherwise.
  public static int[] getVotes (int currentTerm) {
    if (currentTerm == mTerm) {
      return mVotes;
    }
    return null;
  }

  // @param term under which votes are being cleared. method has no
  // effect if the internal term is not equal to the paramter.
  // @return true if votes were cleared, false if not
  public static boolean clearVotes (int currentTerm) {
    if (currentTerm == mTerm) {
      for (int i=0; i<mVotes.length; i++) {
	mVotes[i] = -1;
      } 
      return true;
    }    
    return false;
  }  

  // @param server casting vote.
  // @param return value from RPC to server (0 if server voted for
  // candidate; otherwise, server's current term.
  // @param term under which the vote was cast. method has no effect
  // if it is not equal to the current term.
  // @param round under which the vote was cast. method has no effect
  // if it is not equal to the current round.
  // @return true if vote was set, false if not
  public static boolean setVote (int serverID, 
				 int response, 
				 int voteTerm,
				 int voteRound) {
    if ((voteTerm == mTerm) && (voteRound == mRounds[serverID])) {
      mVotes[serverID] = response;
      return true;
    }
    return false;
  }
  
  // @param current term. 
  // @return null if the internal term not equal to the
  // paramter. array of append responses otherwise.
  public static int[] getAppendResponses (int currentTerm) {
    if (currentTerm == mTerm) {
      return mAppendResponses;
    }    
    return null;
  }

  // @param the current term. method has no effect if the internal
  // term is not equal to the paramter. 
  // @return true if responses were cleared, false if not
  public static boolean clearAppendResponses (int currentTerm) {
    if (currentTerm == mTerm) {
      for (int i=0; i<mAppendResponses.length; i++) {
	mAppendResponses[i] = -1;
      }
      return true;
    }
    return false;
  }  
  
  // @param responding server
  // @param return value from RPC to server
  // @param term under which the request was sent. method has no
  // effect if the request term is not equal to current term.
  // @param round under which the request was sent. method has no
  // effect if the request round is not equal to current round.
  // @return true if response was set, false if not
  public static boolean setAppendResponse (int serverID, 
					   int response, 
					   int requestTerm,
					   int requestRound) {
    if ((requestTerm == mTerm) && (requestRound == mRounds[serverID])) {
      mAppendResponses[serverID] = response;
      return true;      
    } 
    return false;
  }

  // @param current term. 
  // @return null if the internal term not equal to the
  // parameter. array of message rounds.
  public static int[] getRounds (int currentTerm) {
    if (currentTerm == mTerm) {
      return mRounds;
    }    
    return null;
  }
  
  // @param server 
  // @param latest round under which a request was sent to the server
  // @param term under which the request was sent. method has no
  // effect if the request term is not equal to current term or if the
  // new round is less than the old round
  // @return true if response was set, false if not
  public static boolean setRound (int serverID, 
				  int round, 
				  int requestTerm) {
    if ((requestTerm == mTerm) && (round > mRounds[serverID])) {
      mRounds[serverID] = round;
      return true;      
    } 
    return false;
  }
}

  
