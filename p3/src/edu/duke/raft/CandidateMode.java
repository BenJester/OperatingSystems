package edu.duke.raft;
import java.util.Random;
import java.util.Timer;

public class CandidateMode extends RaftMode {
    protected static Timer timer0;
    protected static Timer timer1;
    protected static Random rand = new Random();
    private boolean mIsDead = false;

    private void setTimer() {
        long millis = (long) (rand.nextInt((ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + 1) + ELECTION_TIMEOUT_MIN);
        if (mConfig.getTimeoutOverride() != -1) {
            millis = mConfig.getTimeoutOverride();
        }
        timer0 = scheduleTimer(millis, 0);
    }

    public void go() {
        synchronized(mLock) {
            // increment current term
            RaftResponses.clearVotes(mConfig.getCurrentTerm());
            int term = mConfig.getCurrentTerm() + 1;
            mConfig.setCurrentTerm(term, mID);
            RaftResponses.setTerm(term);
            
            System.out.println("S" + mID + "." + term + ": switched to candidate mode.");

            for(int n=1; n<=mConfig.getNumServers(); n++) {
                if(n != mID) {
                    remoteRequestVote(n, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm()); 
                }
            }

            setTimer();
            timer1 = scheduleTimer(HEARTBEAT_INTERVAL, 1);  // vote request (loop) timer
        }
    }

    // check if enough votes are obtained
    private boolean gotMajority() {
        int[] votes = RaftResponses.getVotes(mConfig.getCurrentTerm());
        int numVotes = 0;
        if(votes == null) {
            return false;
        }
        for(int i=1; i<votes.length; i++) {
            if (i == mID) continue;
            if(votes[i] == 0) {
                numVotes += 1;
            }
        }
        return numVotes+1 > mConfig.getNumServers()/2;
    }

    // check if cadidate's log and index are up-to-date with self
    private boolean upToDate(int lastLogIndex, int lastLogTerm) {
        if(mLog.getLastTerm() != lastLogTerm) {
            return mLog.getLastTerm() < lastLogTerm;
        } else {
            return mLog.getLastIndex() <= lastLogIndex;
        }
    }

    // @param candidate’s term
    // @param candidate requesting vote
    // @param index of candidate’s last log entry
    // @param term of candidate’s last log entry
    // @return 0, if server votes for candidate; otherwise, server's
    // current term 
    public int requestVote(int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
        synchronized(mLock) {
            // System.out.println("S" + mID + "." + mConfig.getCurrentTerm() + ""+ "gets RV as a candidate from " + candidateID);
            if (mIsDead) return mConfig.getCurrentTerm();
            int term = mConfig.getCurrentTerm();
            int result = term;

            if(term >= candidateTerm) {
                return result;
            } else {
                // potentially vote for candidate since its term is greater
                timer0.cancel();
                timer1.cancel();
                RaftResponses.clearVotes(mConfig.getCurrentTerm());
                //RaftResponses.setTerm(candidateTerm);
                result = candidateTerm;

                if(upToDate(lastLogIndex, lastLogTerm)) {
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    // System.out.println("S" + mID + "." + term + " votes for S" + candidateID + " new term: " + candidateID);
                    result = 0;
                } else {
                    mConfig.setCurrentTerm(candidateTerm, 0);
                }
                mIsDead = true;
                RaftMode follower = new FollowerMode();
                RaftServerImpl.setMode(follower);
                return result;
            }
        }
    }
    
    // @param leader’s term
    // @param current leader
    // @param index of log entry before entries to append
    // @param term of log entry before entries to append
    // @param entries to append (in order of 0 to append.length-1)
    // @param index of highest committed entry
    // @return 0, if server appended entries; otherwise, server's
    // current term
    public int appendEntries(int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries, int leaderCommit) {
        synchronized(mLock) {
            if (mIsDead) return mConfig.getCurrentTerm();
            if(mConfig.getCurrentTerm() > leaderTerm) {
                return mConfig.getCurrentTerm();
            } else {
                RaftResponses.clearVotes(mConfig.getCurrentTerm());
                mConfig.setCurrentTerm(leaderTerm, 0);

                timer0.cancel();
                timer1.cancel();
                
                if (entries.length == 0) {
                    if (prevLogIndex == mLog.getLastIndex() && prevLogTerm == mLog.getLastTerm()) { 
                        mIsDead = true;
                        RaftMode follower = new FollowerMode();
                        RaftServerImpl.setMode(follower);
                        return 0;

                    } else {
                        mIsDead = true;
                        RaftMode follower = new FollowerMode();
                        RaftServerImpl.setMode(follower);
                        return leaderTerm;

                    }
                }
                int result = leaderTerm;
                if (mLog.insert(entries, prevLogIndex, prevLogTerm) != -1) {
                    result = 0;
                    // System.out.println("S" + mID + "." + term + "receives appendEntry from " + leaderID);
                }
                mIsDead = true;
                RaftMode follower = new FollowerMode();
                RaftServerImpl.setMode(follower);
                return result;
            }
        }
    }

    // @param id of the timer that timed out
    public void handleTimeout(int timerID) {
        synchronized(mLock) {
            if (mIsDead) return;
            if(timerID == 1) { // vote request timeout
                //System.out.println("S" + mID + "." + mConfig.getCurrentTerm() + " checking if having got majority");
                
                int[] votes = RaftResponses.getVotes(mConfig.getCurrentTerm());
                if (votes != null) {
                        for(int i=1; i<votes.length; i++) {
                        if (i == mID) continue;
                        if(votes[i] > mConfig.getCurrentTerm()) {
                            timer0.cancel();
                            timer1.cancel();
                            RaftResponses.clearVotes(mConfig.getCurrentTerm());
                            mIsDead = true;
                            mConfig.setCurrentTerm(votes[i], 0);
                            RaftMode follower = new FollowerMode();
                            RaftServerImpl.setMode(follower);
                            return;
                        }
                    }
                }


                if(gotMajority()) {
                    timer0.cancel();
                    mIsDead = true;
                    RaftMode leader = new LeaderMode();
                    RaftServerImpl.setMode(leader);
                    return;
                } else {
                    for(int n=1; n<=mConfig.getNumServers(); n++) {
                        if(n != mID) {
                            remoteRequestVote(n, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm()); 
                        }
                    }
                    timer1 = scheduleTimer(HEARTBEAT_INTERVAL, 1);
                }
            } else if(timerID == 0) { // election timeout
                timer1.cancel();
                timer0.cancel();
                RaftResponses.clearVotes(mConfig.getCurrentTerm());
                int term = mConfig.getCurrentTerm() + 1;
                mConfig.setCurrentTerm(term, mID);
                
                RaftResponses.setTerm(term);
                for(int n=1; n<=mConfig.getNumServers(); n++) {
                    if(n != mID) {
                        remoteRequestVote(n, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm()); 
                    }
                }

                setTimer();
                timer1 = scheduleTimer(HEARTBEAT_INTERVAL, 1);  // vote request (loop) timer
            }
        }
    }
}
