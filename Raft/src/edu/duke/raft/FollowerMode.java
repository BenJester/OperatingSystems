package edu.duke.raft;
import java.util.Random;
import java.util.Timer;

public class FollowerMode extends RaftMode {
    protected static Timer timer;
    protected static Random rand = new Random();
    private boolean mIsDead = false;


    private void setTimer() {
        long millis = (long) (rand.nextInt((ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + 1) + ELECTION_TIMEOUT_MIN);
        if (mConfig.getTimeoutOverride() != -1) {
            millis = mConfig.getTimeoutOverride();
        }
        timer = scheduleTimer(millis, 0);
    }


    public void go() {
        synchronized(mLock) {
            int term = mConfig.getCurrentTerm();
            System.out.println("S" + mID + "." + term + ": switched to follower mode.");
            setTimer();
        }
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
            //System.out.println("S" + mID + "." + mConfig.getCurrentTerm() + ""+ "gets RV as a follower from " + candidateID);
            if (mIsDead) return mConfig.getCurrentTerm();

            int term = mConfig.getCurrentTerm();
            if(term > candidateTerm) {
                return term; // nay vote
            } else if (term < candidateTerm) {
                if (upToDate(lastLogIndex, lastLogTerm)) {
                    timer.cancel();
                    setTimer();
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    return 0;
                }
                mConfig.setCurrentTerm(candidateTerm, 0);
                return mConfig.getCurrentTerm();
            } else {
                if ((mConfig.getVotedFor() == 0 || mConfig.getVotedFor() == candidateID) && upToDate(lastLogIndex, lastLogTerm)) {
                    timer.cancel();
                    setTimer();
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    return 0;
                }
                return mConfig.getCurrentTerm();
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

            int term = mConfig.getCurrentTerm();
            if(term > leaderTerm) {
                return term; // nay result
            } else {
                timer.cancel();

                if(term < leaderTerm) {
                    mConfig.setCurrentTerm(leaderTerm, 0);
                    term = leaderTerm;
                }
                if (entries.length == 0) {
                    setTimer();
                    if (prevLogIndex == mLog.getLastIndex() && prevLogTerm == mLog.getLastTerm()) { 
                        return 0;
                    } else {
                        return term;
                    }
                } 
                
                if(mLog.insert(entries, prevLogIndex, prevLogTerm) != -1) {
                    setTimer();
                    return 0; // yay result
                }
                setTimer();
                return term; // nay result
            }
        }
    }

    // @param id of the timer that timed out
    public void handleTimeout(int timerID) {
        synchronized(mLock) {

            if (mIsDead) return;
            timer.cancel();

            mIsDead = true;
            RaftMode candidate = new CandidateMode();
            RaftServerImpl.setMode(candidate);
            return;
        }
    }
}
